#include "Loader.h"
#include "../geometry/Utils.h"
#include "../Parameters.h"

Loader::Loader()
	:mStatus(LoaderThreadStatus::IDLE),
	mRunning(true),
	mLoadJobStatus(LoadJobStatus::FAIL),
	mLoadJobPath(std::string()),
	mLoadBuffer(nullptr)
{
	mLoaderThread = std::thread(&Loader::loaderThreadMain, this);
}

Loader::~Loader()
{
	mRunning = false;
	mCond.notify_one();
	mLoaderThread.join();
}

void Loader::loaderThreadMain()
{
	std::unique_lock<std::mutex> lock(mMutex);
	if (mLoadJobPath == "") {
		mCond.wait(lock);
	}
	while (mRunning) {
		mStatus = LoaderThreadStatus::LOADING;

		bool result = loadSceneObjectSynchronous(mLoadJobPath, mLoadBuffer);

		if (result) {
			mLoadJobStatus = LoadJobStatus::SUCCESS;
		}
		else {
			mLoadJobStatus = LoadJobStatus::FAIL;
		}
		mStatus = LoaderThreadStatus::FINISHED;
		mCond.wait(lock);
	}
	lock.unlock();
}

bool Loader::loadSceneObjectAsynch(std::string object_path, SceneObject* scene_object)
{
	if (mStatus == LoaderThreadStatus::LOADING) {
		std::cout << "Error, Already loading an object" << std::endl;
		return false;
	}

	std::unique_lock<std::mutex> lock(mMutex);
	mLoadBuffer = scene_object;
	mLoadJobPath = object_path;
	lock.unlock();
	mCond.notify_one();

	return true;
}

bool Loader::loadSceneObjectSynchronous(std::string object_path, SceneObject* scene_object)
{
	if (!std::filesystem::exists(object_path)) {
		std::cout << "Error: Can't find object file: " << object_path << std::endl;
		return false;
	}

	Mesh_Object_t mesh;

	std::filesystem::path path(object_path);
	std::string filename = path.filename().string();

	std::string dir = path.remove_filename().string();
	bool res = loadObjMesh(dir, filename, mesh, ShadingType::FLAT);

	glm::vec3 diff = mesh.higher - mesh.lower;
	float max_dimension = glm::max(diff.x, glm::max(diff.y, diff.z));

	float max_edge_length = max_dimension / (glm::pow(2, parameters.max_tree_depth));
	max_edge_length = 0.1f;

	bool tesselation_res = tesselateTriforce(mesh, max_edge_length, -1);

	if (!res) {
		std::cout << "Failed to load obj file: " << dir + filename << std::endl;
		return false;
	}

	scene_object->setMeshObject(mesh);

	std::shared_ptr<VoxelGrid> voxels;
	if (initVoxels(mesh, parameters.voxel_count, voxels)) {
		scene_object->registerVoxels(voxels);
	}
	else {
		std::cout << "Failed to load voxels for obj file: " << dir + filename << std::endl;
	}

	std::shared_ptr<Octree> octree = std::make_shared<Octree>();
	bool octree_res = build_Obj_Octree(mesh, *octree, parameters.max_tree_depth);
	if (!octree_res) {
		std::cout << "Error, failing at attempting to build octree" << std::endl;
	}
	else {
		octree->createSGGX();

		scene_object->registerOctree(octree);
	}

	return true;
}

const LoaderThreadStatus Loader::getThreadStatus()
{
	return mStatus;
}

const LoadJobStatus Loader::getJobStatus()
{
	return mLoadJobStatus;
}

void Loader::receivedResult()
{
	if (mStatus == LoaderThreadStatus::FINISHED) {
		mStatus = LoaderThreadStatus::IDLE;
	}
}

std::shared_ptr<RasterizationObject> Loader::registerMeshObject(Mesh_Object_t& source, std::shared_ptr<Shader> shader, glm::mat4 model_matrix)
{
	shader->bind();
	std::shared_ptr<VertexArray> va = std::make_shared<VertexArray>();


	std::shared_ptr<ArrayBuffer> vertices = std::make_shared<ArrayBuffer>(source.vertices.data(), source.vertices.size() * sizeof(float));
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);


	std::shared_ptr<ArrayBuffer> normals = std::make_shared<ArrayBuffer>(source.normals.data(), source.normals.size() * sizeof(float));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(1);

	//if (source.tex_coords.size() != 0) {
	std::shared_ptr<ArrayBuffer> tex_coords = std::make_shared<ArrayBuffer>(source.tex_coords.data(), source.tex_coords.size() * sizeof(float));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glEnableVertexAttribArray(2);

	size_t s = sizeof(unsigned int);

	std::map<unsigned int, unsigned int> counts;
	for (auto& val : source.face_material_indices) {
		auto res = counts.find(val);
		if (res == counts.end()) {
			counts[val] = 1;
		}
		else {
			counts[val]++;
		}
	}

	const size_t glsl_uint_size = sizeof(GL_UNSIGNED_INT);
	std::shared_ptr<ArrayBuffer> face_mat_indices = std::make_shared<ArrayBuffer>(source.face_material_indices.data(), source.face_material_indices.size() * glsl_uint_size);
	glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, glsl_uint_size, 0);
	glEnableVertexAttribArray(3);

	std::shared_ptr<IndexBuffer> ib = std::make_shared<IndexBuffer>(source.indices.data(), source.indices.size());
	std::shared_ptr<RasterizationObject> object = std::make_shared<RasterizationObject>(va, ib, shader, model_matrix, source.materials);

	object->addArrayBuffer(vertices);
	object->addArrayBuffer(normals);
	object->addArrayBuffer(tex_coords);
	object->addArrayBuffer(face_mat_indices);

	for (int i = 0; i < object->getMaterials().size(); i++) {
		auto mat = object->getMaterials()[i];
		if (mat.diffuse_texname != "") {
			std::shared_ptr<Texture> tex = std::make_shared<Texture>(mat.diffuse_texname);
			object->addTexture(tex);
		}
	}
	shader->unbind();
	return object;
}

bool Loader::initVoxels(Mesh_Object_t& obj, unsigned int dimension, std::shared_ptr<VoxelGrid>& voxels)
{
	voxels = std::make_shared<VoxelGrid>(dimension);

	std::cout << "Initializing Voxels" << std::endl;
	voxels->setLower(obj.lower);
	voxels->setHigher(obj.higher);
	float voxelSize = voxels->getVoxelSize();
	//auto dimension = voxels->getDimension();
	/*
	for (uint16_t x = 0; x < dimension; x++) {
		for (uint16_t y = 0; y < dimension; y++) {
			for (uint16_t z = 0; z < dimension; z++) {
				glm::u16vec3 idx_vec = glm::u16vec3(x, y, z);
				uint16_t dist = glm::floor(glm::length(glm::vec3(idx_vec) - glm::vec3(dimension / 2)));
				float len = glm::sqrt(idx_vec.x* idx_vec.x + idx_vec.y * idx_vec.y + idx_vec.z * idx_vec.z);

				float lower = 5;
				if (dist > lower && dist <= lower + 1) {
					voxels->setVoxel(idx_vec, { 1 });
				}
				else {
					voxels->setVoxel(idx_vec, { 0 });
				}
			}
		}
	}
	*/
	unsigned int count = 0;
	for (uint32_t idx = 0; idx < obj.indices.size(); idx += 3) {
		if (idx % 3000 == 0) {
			std::cout << "voxelizing triangle #" << idx / 3 << std::endl;
		}
		glm::vec3 v1 = glm::vec3(obj.vertices[3 * obj.indices[idx]], obj.vertices[3 * obj.indices[idx] + 1], obj.vertices[3 * obj.indices[idx] + 2]);
		glm::vec3 v2 = glm::vec3(obj.vertices[3 * obj.indices[idx + 1]], obj.vertices[3 * obj.indices[idx + 1] + 1], obj.vertices[3 * obj.indices[idx + 1] + 2]);
		glm::vec3 v3 = glm::vec3(obj.vertices[3 * obj.indices[idx + 2]], obj.vertices[3 * obj.indices[idx + 2] + 1], obj.vertices[3 * obj.indices[idx + 2] + 2]);

		glm::vec3 n1 = glm::vec3(obj.normals[3 * obj.indices[idx]], obj.normals[3 * obj.indices[idx] + 1], obj.normals[3 * obj.indices[idx] + 2]);
		glm::vec3 n2 = glm::vec3(obj.normals[3 * obj.indices[idx + 1]], obj.normals[3 * obj.indices[idx + 1] + 1], obj.normals[3 * obj.indices[idx + 1] + 2]);
		glm::vec3 n3 = glm::vec3(obj.normals[3 * obj.indices[idx + 2]], obj.normals[3 * obj.indices[idx + 2] + 1], obj.normals[3 * obj.indices[idx + 2] + 2]);

		glm::vec3 center = (v1 + v2 + v3) / glm::vec3(3);
		glm::vec3 normal = (n1 + n2 + n3) / glm::vec3(3);

		glm::u16vec3 idx_vec = glm::u16vec3(glm::floor((center - voxels->getLower()) / voxels->getVoxelSize()));
		if (idx_vec.x >= 0 && idx_vec.x < dimension
			&& idx_vec.y >= 0 && idx_vec.y < dimension
			&& idx_vec.z >= 0 && idx_vec.z < dimension) {
			auto voxel = voxels->getVoxel(idx_vec);
			float density = glm::min(0.03f + voxel.val, 1.0f);
			glm::vec3 n = normal + glm::vec3(voxel.normal_x, voxel.normal_y, voxel.normal_z);

			voxels->setVoxel(idx_vec, { density, normal.x, normal.y, normal.z });
		}
	}

	//voxels->initBuffers();

	unsigned int voxel_count = voxels->countVoxels();
	unsigned int non_empty_voxels = voxels->countNonEmptyVoxels();
	std::cout << "Voxel count: " << voxel_count << "\n";
	std::cout << "Non empty Voxel count: " << non_empty_voxels << "\n";
	std::cout << "Abs difference: " << (voxel_count - non_empty_voxels) << "\n";
	std::cout << "Voxelgrid density: " << ((float)non_empty_voxels) / voxel_count << std::endl;

	return true;
}