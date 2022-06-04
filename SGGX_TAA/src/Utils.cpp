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

	for (size_t i = 0; i < indices.size(); i++) {

		max_index = glm::max(indices[i].vertex_index, max_index);

		target.vertices.push_back(vertices[3 * indices[i].vertex_index]);
		target.vertices.push_back(vertices[3 * indices[i].vertex_index + 1]);
		target.vertices.push_back(vertices[3 * indices[i].vertex_index + 2]);
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

	std::cout << "Vertex tex_coords" << "\n" << std::endl;
	for (int i = 0; i < max_index; i++) {
		int size = tex_map[i].size();
		std::cout << "Size: " << size << "\t";
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
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;

	std::string obj_path = model_path + model_name;

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

	return true;
}