#include "Utils.h"

//tmp
#include <glm/glm.hpp>

void make_flat_shaded(std::vector<float>& vertices,
	std::vector<float>& normals,
	std::vector<float>& colors,
	std::vector<float>& tex_coords,
	std::vector<tinyobj::index_t>& indices,
	Mesh_Object_t& target) {

	std::map<unsigned int, std::vector<unsigned int>> tex_map;
	int max_index = 0;
	target.lower = glm::vec3(INFINITY);
	target.higher = glm::vec3(-INFINITY);

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
	}

	//std::cout << "Handling Vertex tex_coords" << "\n" << std::endl;
	/*
	for (int i = 0; i < max_index; i++) {
		int size = tex_map[i].size();
		/*
		//std::cout << "Size: " << size << "\t";
		for (int j = 0; j < size; j++) {
			std::cout << " " << tex_map[i][j] << ",";
		}
		std::cout << std::endl;
		
		if (size > 10) {

			std::cout << "X: " << vertices[3 * i] << std::endl;
			std::cout << "Y: " << vertices[3 * i + 1] << std::endl;
			std::cout << "Z: " << vertices[3 * i + 2] << std::endl;
		}

	}
	*/
}

void make_smooth_shaded(std::vector<float>& vertices,
	std::vector<float>& normals,
	std::vector<float>& colors,
	std::vector<float>& tex_coords,
	std::vector<tinyobj::index_t>& indices,
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
		make_flat_shaded(attrib.vertices, attrib.normals, attrib.colors, attrib.texcoords, shape.mesh.indices, target);
	}
	else if (shadingType == ShadingType::SMOOTH) {
		make_smooth_shaded(attrib.vertices, attrib.normals, attrib.colors, attrib.texcoords, shape.mesh.indices, target);
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

	size_t materials_size = 0;
	fin.read((char*)&materials_size, sizeof(size_t));

	fin.read((char*)glm::value_ptr(target.lower), sizeof(float) * 3);
	fin.read((char*)glm::value_ptr(target.higher), sizeof(float) * 3);

	fin.read((char*)&target.shadingType, sizeof(target.shadingType));

	if (vertices_size > 0) {
		float* vertices_buffer = new float[vertices_size];
		fin.read((char*)vertices_buffer, vertices_size * sizeof(float));
		std::vector<float> vertices(vertices_buffer, vertices_buffer + vertices_size);
		delete vertices_buffer;
		target.vertices = vertices;
	}

	if (tex_coords_size > 0) {
		float* tex_coords_buffer = new float[tex_coords_size];
		fin.read((char*)tex_coords_buffer, tex_coords_size * sizeof(float));
		std::vector<float> tex_coords(tex_coords_buffer, tex_coords_buffer + tex_coords_size);
		delete tex_coords_buffer;
		target.tex_coords = tex_coords;
	}

	if (normals_size > 0) {
		float* normals_buffer = new float[normals_size];
		fin.read((char*)normals_buffer, normals_size * sizeof(float));
		std::vector<float> normals(normals_buffer, normals_buffer + normals_size);
		delete normals_buffer;
		target.normals = normals;
	}

	if (colors_size > 0) {
		float* colors_buffer = new float[colors_size];
		fin.read((char*)colors_buffer, colors_size * sizeof(float));
		std::vector<float> colors(colors_buffer, colors_buffer + colors_size);
		delete colors_buffer;
		target.colors = colors;
	}

	if (indices_size > 0) {
		unsigned int* indices_buffer = new unsigned int[indices_size];
		fin.read((char*)indices_buffer, indices_size * sizeof(unsigned int));
		std::vector<unsigned int> indices(indices_buffer, indices_buffer + indices_size);
		delete indices_buffer;
		target.indices = indices;
	}

	if (materials_size > 0) {
		serializable_material* materials_buffer = new serializable_material[materials_size];
		fin.read((char*)materials_buffer, materials_size * sizeof(serializable_material));
		std::vector<serializable_material> serialized_materials(materials_buffer, materials_buffer + materials_size);
		delete materials_buffer;
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

