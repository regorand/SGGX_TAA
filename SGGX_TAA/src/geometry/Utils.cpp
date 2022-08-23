#include "Utils.h"

//tmp
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

void make_flat_shaded(std::vector<float>& vertices,
	std::vector<float>& normals,
	std::vector<float>& colors,
	std::vector<float>& tex_coords,
	std::vector<tinyobj::index_t>& indices, 
	std::vector<int> face_material_indices,
	Mesh_Object_t& target) {

	std::map<unsigned int, std::vector<unsigned int>> tex_map;
	int max_index = 0;
	target.lower = glm::vec3(INFINITY);
	target.higher = glm::vec3(-INFINITY);

	bool writeMaterialIndices = false;
	if (face_material_indices.size() == indices.size() / 3) {
		writeMaterialIndices = true;
	}

	std::cout << "Handling indices in flat_shaded" << std::endl;
	for (size_t i = 0; i < indices.size(); i++) {

		max_index = glm::max(indices[i].vertex_index, max_index);

		float x_val = vertices[3 * indices[i].vertex_index];
		float y_val = vertices[3 * indices[i].vertex_index + 1];
		float z_val = vertices[3 * indices[i].vertex_index + 2];
		target.vertices.push_back(x_val);
		target.vertices.push_back(y_val);
		target.vertices.push_back(z_val);
		
		target.lower.x = glm::min(target.lower.x, x_val);
		target.higher.x = glm::max(target.higher.x, x_val);
		
		target.lower.y = glm::min(target.lower.y, y_val);
		target.higher.y = glm::max(target.higher.y, y_val);
		
		target.lower.z = glm::min(target.lower.z, z_val);
		target.higher.z = glm::max(target.higher.z, z_val);

		target.normals.push_back(normals[3 * indices[i].normal_index]);
		target.normals.push_back(normals[3 * indices[i].normal_index + 1]);
		target.normals.push_back(normals[3 * indices[i].normal_index + 2]);

		target.colors.push_back(1);
		target.colors.push_back(1);
		target.colors.push_back(1);
		target.colors.push_back(1);

		target.tex_coords.push_back(tex_coords[2 * indices[i].texcoord_index]);
		target.tex_coords.push_back(tex_coords[2 * indices[i].texcoord_index + 1]);

		tex_map[indices[i].vertex_index].push_back(indices[i].texcoord_index);

		target.indices.push_back(i);
		if (writeMaterialIndices) target.face_material_indices.push_back(face_material_indices[i / 3]);

	}
}

void make_smooth_shaded(std::vector<float>& vertices,
	std::vector<float>& normals,
	std::vector<float>& colors,
	std::vector<float>& tex_coords,
	std::vector<tinyobj::index_t>& indices,
	std::vector<int> face_material_indices,
	Mesh_Object_t& target) {

	struct vertex_attrib_s {
		glm::vec3 normal;
		unsigned int tex_coords; // this is not totally correct, edge case vertices may have more than one tex coord (e.g. Pole from a sphere)
	};

	target.vertices = vertices;
	std::unordered_map<unsigned int, vertex_attrib_s> vertex_attribs;

	unsigned int count = 0;
	unsigned int largest_index = 0;
	for (size_t i = 0; i < indices.size(); i++) {
		unsigned int vertex_index = indices[i].vertex_index;
		largest_index = glm::max(vertex_index, largest_index);
		auto search = vertex_attribs.find(vertex_index);
		if (search == vertex_attribs.end()) {
			vertex_attribs[vertex_index].normal = glm::vec3(0);
		}
		glm::vec3 normal(normals[3 * indices[i].normal_index],
			normals[3 * indices[i].normal_index + 1],
			normals[3 * indices[i].normal_index + 2]);
		vertex_attribs[vertex_index].normal += normal;
		count++;

		vertex_attribs[vertex_index].tex_coords = indices[i].texcoord_index;
		// TODO properly do tex coords etc.
		// target.tex_coords.push_back()
		// how tf ?

		target.indices.push_back(indices[i].vertex_index);
	}

	for (unsigned int i = 0; i <= largest_index; i++) {

		target.colors.push_back(1);
		target.colors.push_back(1);
		target.colors.push_back(1);
		target.colors.push_back(1);


		auto entry = vertex_attribs.find(i);

		target.tex_coords.push_back(tex_coords[entry->second.tex_coords * 2]);
		target.tex_coords.push_back(tex_coords[entry->second.tex_coords * 2 + 1]);

		if (entry != vertex_attribs.end()) {
			glm::vec3 normal = entry->second.normal;
			if (normal != glm::vec3(0)) {
				glm::vec3 normalized = glm::normalize(normal);
				target.normals.push_back(normalized.x);
				target.normals.push_back(normalized.y);
				target.normals.push_back(normalized.z);
			}
			else {
				target.normals.push_back(0);
				target.normals.push_back(-1);
				target.normals.push_back(0);
			}
		}
	}
}

bool build_Obj_Octree(Mesh_Object_t& source, Octree &tree, size_t max_depth)
{
	if (source.vertices.size() % 3 != 0) {
		std::cout << "Error, invalid size for vertices Array" << std::endl;
		return false;
	}

	glm::vec3 halfway = (source.lower + source.higher) / glm::vec3(2);
	
	// create single leaf node as root
	tree.init(source.lower, source.higher);
	
	// assign root node all data
	//obj_leaf_node *root_leaf_ptr = nullptr;
	//bool success = tree.getLeafAt(halfway, &root_leaf_ptr);
	//if (!success) return false;
	//
	//root_leaf_ptr->vertices = source.vertices;
	//root_leaf_ptr->normals = source.normals;
	//root_leaf_ptr->indices = source.indices;
	
	size_t maxPointsPerLeaf = 4;
	
	// build tree with given data
	bool build_success = tree.build(max_depth, maxPointsPerLeaf, source);
	if (!build_success) {
		std::cout << "Error: Build of Octree failed" << std::endl;
	}

	return true;
}


bool loadObjMesh(std::string& model_path, std::string& model_name, Mesh_Object_t& target, const ShadingType shadingType) {
	std::string path = model_path + model_name + ".dat";
	if (std::filesystem::exists(path)) {
		std::cout << "Found dat file, trying to load that" << std::endl;
		bool res = readObjectFromFile(model_path + model_name, target);
		if (res) {
			std::cout << "Successfully loaded from .dat file" << std::endl;
			return true;
		}
		std::cout << "Loading from .dat file failed, loading from .obj file" << std::endl;
	}

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;

	std::string obj_path = model_path + model_name;

	std::cout << "tinyobj call" << std::endl;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, obj_path.c_str(), model_path.c_str());

	if (!ret) {
		std::cout << "Error importing obj file: \"" << obj_path << "\"\n" << err << std::endl;
		return false;
	}
	if (shapes.size() == 0) {
		std::cout << "Error importing obj file: \"" << obj_path << "\"\n" << "No Shape found in obj File" << std::endl;
		return false;
	}

	tinyobj::shape_t shape = shapes[0];

	if (shadingType == ShadingType::FLAT) {
		//make_flat_shaded(attrib.vertices, attrib.normals, attrib.colors, attrib.texcoords, shape.mesh.indices, target);
		make_flat_shaded(attrib.vertices, 
			attrib.normals, 
			attrib.colors, 
			attrib.texcoords, 
			shape.mesh.indices, 
			shape.mesh.material_ids, 
			target);
	}
	else if (shadingType == ShadingType::SMOOTH) {
		make_smooth_shaded(attrib.vertices,
			attrib.normals, attrib.colors,
			attrib.texcoords,
			shape.mesh.indices,
			shape.mesh.material_ids,
			target);
	}

	for (auto& mat : materials) {
		glm::vec3 KA = glm::vec3(mat.ambient[0], mat.ambient[1], mat.ambient[2]);
		glm::vec3 KD = glm::vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
		glm::vec3 KS = glm::vec3(mat.specular[0], mat.specular[1], mat.specular[2]);

		Material material(KA, KD, KS, mat.shininess);

		if (mat.diffuse_texname != "") {
			material.diffuse_texname = model_path + mat.diffuse_texname;
		}
		target.materials.push_back(material);
	}

	std::cout << "Trying to save model in .dat file" << std::endl;
	bool res = saveToFile(model_path + model_name, target);
	if (!res) {
		std::cout << "Failed to save model in .dat file" << std::endl;
	}

	return true;
}

// If this functions is changed, always change file version 
bool readObjectFromFile(std::string file_name, Mesh_Object_t &target)
{
	std::string path = file_name + ".dat";
	if (!std::filesystem::exists(path)) {
		std::cout << "File to load doesn't exist: " << file_name << std::endl;
		return false;
	}

	std::ifstream fin(path, std::ios::in | std::ios::binary);

	uint32_t version = 0;
	fin.read((char*)&version, sizeof(version));
	if (version != OBJ_FILE_FORMAT_VERSION) {
		std::cout << "Error: Found .dat file, but it is an older version" << std::endl;
		return false;
	}

	size_t vertices_size = 0;
	fin.read((char*)&vertices_size, sizeof(size_t));

	size_t tex_coords_size = 0;
	fin.read((char*)&tex_coords_size, sizeof(size_t));

	size_t normals_size = 0;
	fin.read((char*)&normals_size, sizeof(size_t));

	size_t colors_size = 0;
	fin.read((char*)&colors_size, sizeof(size_t));

	size_t indices_size = 0;
	fin.read((char*)&indices_size, sizeof(size_t));

	size_t face_mat_indices_size = 0;
	fin.read((char*)&face_mat_indices_size, sizeof(size_t));

	size_t materials_size = 0;
	fin.read((char*)&materials_size, sizeof(size_t));

	fin.read((char*)glm::value_ptr(target.lower), sizeof(float) * 3);
	fin.read((char*)glm::value_ptr(target.higher), sizeof(float) * 3);

	fin.read((char*)&target.shadingType, sizeof(target.shadingType));

	if (vertices_size > 0) {
		float* vertices_buffer = new float[vertices_size];
		fin.read((char*)vertices_buffer, vertices_size * sizeof(float));
		std::vector<float> vertices(vertices_buffer, vertices_buffer + vertices_size);
		delete[] vertices_buffer;
		target.vertices = vertices;
	}

	if (tex_coords_size > 0) {
		float* tex_coords_buffer = new float[tex_coords_size];
		fin.read((char*)tex_coords_buffer, tex_coords_size * sizeof(float));
		std::vector<float> tex_coords(tex_coords_buffer, tex_coords_buffer + tex_coords_size);
		delete[] tex_coords_buffer;
		target.tex_coords = tex_coords;
	}

	if (normals_size > 0) {
		float* normals_buffer = new float[normals_size];
		fin.read((char*)normals_buffer, normals_size * sizeof(float));
		std::vector<float> normals(normals_buffer, normals_buffer + normals_size);
		delete[] normals_buffer;
		target.normals = normals;
	}

	if (colors_size > 0) {
		float* colors_buffer = new float[colors_size];
		fin.read((char*)colors_buffer, colors_size * sizeof(float));
		std::vector<float> colors(colors_buffer, colors_buffer + colors_size);
		delete[] colors_buffer;
		target.colors = colors;
	}

	if (indices_size > 0) {
		unsigned int* indices_buffer = new unsigned int[indices_size];
		fin.read((char*)indices_buffer, indices_size * sizeof(unsigned int));
		std::vector<unsigned int> indices(indices_buffer, indices_buffer + indices_size);
		delete[] indices_buffer;
		target.indices = indices;
	}

	if (face_mat_indices_size > 0) {
		unsigned int* face_mat_indices_buffer = new unsigned int[face_mat_indices_size];
		fin.read((char*)face_mat_indices_buffer, face_mat_indices_size * sizeof(unsigned int));
		std::vector<unsigned int> face_mat_indices(face_mat_indices_buffer, face_mat_indices_buffer + face_mat_indices_size);
		delete[] face_mat_indices_buffer;
		target.face_material_indices = face_mat_indices;
	}

	if (materials_size > 0) {
		serializable_material* materials_buffer = new serializable_material[materials_size];
		fin.read((char*)materials_buffer, materials_size * sizeof(serializable_material));
		std::vector<serializable_material> serialized_materials(materials_buffer, materials_buffer + materials_size);
		delete[] materials_buffer;
		target.materials.resize(0);
		std::transform(serialized_materials.begin(),
			serialized_materials.end(),
			std::back_inserter(target.materials),
			[](serializable_material s) {
				return Material(s);
			});
	}

	return true;
}

bool saveToFile(std::string file_name, Mesh_Object_t& object)
{
	std::string path = file_name + ".dat";
	if (std::filesystem::exists(path)) {
		return false;
	}

	std::ofstream fout(path, std::ios::out | std::ios::binary);

	fout.write((char*)&OBJ_FILE_FORMAT_VERSION, sizeof(OBJ_FILE_FORMAT_VERSION));

	size_t vertices_size = object.vertices.size();
	fout.write((char*) &vertices_size, sizeof(size_t));

	size_t tex_coords_size = object.tex_coords.size();
	fout.write((char*) &tex_coords_size, sizeof(size_t));

	size_t normals_size = object.normals.size();
	fout.write((char*) &normals_size, sizeof(size_t));

	size_t colors_size = object.colors.size();
	fout.write((char*) &colors_size, sizeof(size_t));

	size_t indices_size = object.indices.size();
	fout.write((char*) &indices_size, sizeof(size_t));

	size_t face_mat_indices_size = object.face_material_indices.size();
	fout.write((char*)&face_mat_indices_size, sizeof(size_t));

	size_t materials_size = object.materials.size();
	fout.write((char*)&materials_size, sizeof(size_t));

	fout.write((char*)glm::value_ptr(object.lower), sizeof(float) * 3);
	fout.write((char*)glm::value_ptr(object.higher), sizeof(float) * 3);

	fout.write((char*) &object.shadingType, sizeof(object.shadingType));

	fout.write((char*) object.vertices.data(), vertices_size * sizeof(float));
	fout.write((char*) object.tex_coords.data(), tex_coords_size * sizeof(float));
	fout.write((char*) object.normals.data(), normals_size * sizeof(float));
	fout.write((char*) object.colors.data(), colors_size * sizeof(float));
	fout.write((char*) object.indices.data(), indices_size * sizeof(unsigned int));
	fout.write((char*) object.face_material_indices.data(), face_mat_indices_size * sizeof(unsigned int));

	std::vector<serializable_material> serializable_mats;

	std::transform(object.materials.begin(),
		object.materials.end(),
		std::back_inserter(serializable_mats),
		[](Material m) { 
			serializable_material s_mat;
			m.serialize(s_mat);
			return s_mat;
		});

	fout.write((char*)serializable_mats.data(), materials_size * sizeof(serializable_material));
	fout.close();

	return true;
}

bool tesselateTriforce(Mesh_Object_t& object, float max_edge_length, int max_iteration)
{
	if (object.indices.size() % 3 != 0) return false;

	bool anotherIteration = true;

	size_t count = 0;
	int iteration_count = 0;

	while (anotherIteration && (max_iteration == -1 || iteration_count < max_iteration)) {
		iteration_count++;
		anotherIteration = false;
		size_t max_iters = object.indices.size();
		for (size_t i = 0; i < max_iters; i+=3) {
		//for (size_t i = 0; i < object.indices.size(); i += 3) {
			glm::vec3 v1 = glm::make_vec3(object.vertices.data() + 3 * object.indices[i]);
			glm::vec3 v2 = glm::make_vec3(object.vertices.data() + 3 * object.indices[i + 1]);
			glm::vec3 v3 = glm::make_vec3(object.vertices.data() + 3 * object.indices[i + 2]);
			
			float l12 = glm::length(v1 - v2);
			float l23 = glm::length(v2 - v3);
			float l31 = glm::length(v3 - v1);

			float max_length = glm::max(l12, glm::max(l23, l31));

			if (max_length > max_edge_length) {
				count++;
				anotherIteration = true;

				unsigned int v1_index = object.indices[i];
				unsigned int v2_index = object.indices[i + 1];
				unsigned int v3_index = object.indices[i + 2];
				// Step 1: Create new Vertices
				glm::vec3 v12 = (v1 + v2) / glm::vec3(2);
				glm::vec3 v23 = (v2 + v3) / glm::vec3(2);
				glm::vec3 v31 = (v3 + v1) / glm::vec3(2);

				unsigned int old_vertices_size = object.vertices.size();
				
				unsigned int v12_index = (old_vertices_size / 3);
				unsigned int v23_index = (old_vertices_size / 3) + 1;
				unsigned int v31_index = (old_vertices_size / 3) + 2;

				object.vertices.push_back(v12.x);
				object.vertices.push_back(v12.y);
				object.vertices.push_back(v12.z);

				object.vertices.push_back(v23.x);
				object.vertices.push_back(v23.y);
				object.vertices.push_back(v23.z);

				object.vertices.push_back(v31.x);
				object.vertices.push_back(v31.y);
				object.vertices.push_back(v31.z);

				// Step 3: Copy over other properties from vertices:
				//		-> Normals, colors, etc.

				object.tex_coords.push_back(object.tex_coords[2 * v1_index]);
				object.tex_coords.push_back(object.tex_coords[2 * v1_index + 1]);

				object.tex_coords.push_back(object.tex_coords[2 * v2_index]);
				object.tex_coords.push_back(object.tex_coords[2 * v2_index + 1]);

				object.tex_coords.push_back(object.tex_coords[2 * v3_index]);
				object.tex_coords.push_back(object.tex_coords[2 * v3_index + 1]);

				object.normals.push_back(object.normals[3 * v1_index]);
				object.normals.push_back(object.normals[3 * v1_index + 1]);
				object.normals.push_back(object.normals[3 * v1_index + 2]);

				object.normals.push_back(object.normals[3 * v2_index]);
				object.normals.push_back(object.normals[3 * v2_index + 1]);
				object.normals.push_back(object.normals[3 * v2_index + 2]);

				object.normals.push_back(object.normals[3 * v3_index]);
				object.normals.push_back(object.normals[3 * v3_index + 1]);
				object.normals.push_back(object.normals[3 * v3_index + 2]);

				// Also do Colors ? -> No, may not even be defined

				object.face_material_indices.push_back(object.face_material_indices[v1_index]);
				object.face_material_indices.push_back(object.face_material_indices[v2_index]);
				object.face_material_indices.push_back(object.face_material_indices[v3_index]);

				// Step 3: Update current indices and add new indices

				// First triangle: update current indices
				object.indices[i + 1] = v12_index;
				object.indices[i + 2] = v31_index;

				// Second triangle: v12, v23, v31
				object.indices.push_back(v12_index);
				object.indices.push_back(v23_index);
				object.indices.push_back(v31_index);

				// Third triangle: v12, v2, v23
				object.indices.push_back(v12_index);
				object.indices.push_back(v2_index);
				object.indices.push_back(v23_index);

				// Fourth Triangle: v23, v3, v31
				object.indices.push_back(v23_index);
				object.indices.push_back(v3_index);
				object.indices.push_back(v31_index);
			}
		}
	}
	return true;
}

bool calculateClampedBarycentricCoordinates(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 p, float& u, float& v, float& w)
{
	glm::vec3 v0 = b - a, v1 = c - a, v2 = p - a;
	float d00 = glm::dot(v0, v0);
	float d01 = glm::dot(v0, v1);
	float d11 = glm::dot(v1, v1);
	float d20 = glm::dot(v2, v0);
	float d21 = glm::dot(v2, v1);
	float denom = d00 * d11 - d01 * d01;
	v = (d11 * d20 - d01 * d21) / denom;
	w = (d00 * d21 - d01 * d20) / denom;
	u = 1.0f - v - w;

	if (u < 0) u = 0;
	if (v < 0) v = 0;
	if (w < 0) w = 0;

	float sum = u + v + w;
	
	u /= sum;
	v /= sum;
	w /= sum;


	/*
	if (u < 0)
	{
		float dot1 = glm::dot(p - b, c - b);
		float dot2 = glm::dot(c - b, c - b);
		float t = glm::dot(p - b, c - b) / glm::dot(c - b, c - b);
		t = clamp01(t);
		u = 0.0f;
		v = 1.0f - t;
		w = t;
	}
	else if (v < 0)
	{
		float t = glm::dot(p - c, a - c) / glm::dot(a - c, a - c);
		t = clamp01(t);
		u = t;
		v = 0.0f;
		w = 1.0f - t;
	}
	else if (w < 0)
	{
		float t = glm::dot(p - a, b - a) / glm::dot(b - a, b - a);
		t = clamp01(t);
		u = 1.0f - t;
		v = t; 
		w = 0.0f;
	}
	*/

	return true;
}

bool triangleBoxIntersection(AABB box, Triangle tri)
{


	// appearently the plural of "axis" is "axes"
	std::vector<glm::vec3> axes;

	// normals of the aabb
	glm::vec3 u0 = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 u1 = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 u2 = glm::vec3(0.0f, 0.0f, 1.0f);

	// edge directions of triangle
	glm::vec3 e0 = glm::normalize(tri.v0 - tri.v1);
	glm::vec3 e1 = glm::normalize(tri.v1 - tri.v2);
	glm::vec3 e2 = glm::normalize(tri.v2 - tri.v0);

	// 9 axes are the cross products of triangle edges and aabb normals
	axes.push_back(glm::cross(e0, u0));
	axes.push_back(glm::cross(e0, u1));
	axes.push_back(glm::cross(e0, u2));

	axes.push_back(glm::cross(e1, u0));
	axes.push_back(glm::cross(e1, u1));
	axes.push_back(glm::cross(e1, u2));

	axes.push_back(glm::cross(e2, u0));
	axes.push_back(glm::cross(e2, u1));
	axes.push_back(glm::cross(e2, u2));

	// aabbs normals are 3 axes to check
	axes.push_back(u0);
	axes.push_back(u1);
	axes.push_back(u2);

	//triangle normal is last 
	axes.push_back(glm::cross(tri.v0, tri.v1));

	for (auto& axis : axes) {
		if (!axisIntersection(box, tri, axis)) {
			return false;
		}
	}

	return true;
}

bool axisIntersection(AABB box, Triangle tri, glm::vec3 axis)
{
	// Code taken and modified from 
	// https://gdbooks.gitbooks.io/3dcollisions/content/Chapter4/aabb-triangle.html

	tri.v0 -= box.center;
	tri.v1 -= box.center;
	tri.v2 -= box.center;

	glm::vec3 u0 = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 u1 = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 u2 = glm::vec3(0.0f, 0.0f, 1.0f);

	float p0 = glm::dot(tri.v0, axis);
	float p1 = glm::dot(tri.v1, axis);
	float p2 = glm::dot(tri.v2, axis);
	// Project the AABB onto the seperating axis
	// We don't care about the end points of the prjection
	// just the length of the half-size of the AABB
	// That is, we're only casting the extents onto the 
	// seperating axis, not the AABB center. We don't
	// need to cast the center, because we know that the
	// aabb is at origin compared to the triangle!
	float r =	box.extents.x * glm::abs(glm::dot(u0, axis)) +
				box.extents.y * glm::abs(glm::dot(u1, axis)) +
				box.extents.z * glm::abs(glm::dot(u2, axis));
	// Now do the actual test, basically see if either of
	// the most extreme of the triangle points intersects r
	// You might need to write Min & Max functions that take 3 arguments

	return glm::max(-max3(p0, p1, p2), min3(p0, p1, p2)) <= r;
}
