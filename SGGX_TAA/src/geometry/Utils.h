#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <filesystem>
#include <glm/gtc/type_ptr.hpp>
//#include <glm/gtc/type_ptr.hpp >

#include "mesh_object.h"
#include "glm/glm.hpp"
// #include "../Renderer/Material.h"

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "../3rd_party/obj/tiny_obj_loader.h"

#include "../3rd_party/std_image/std_image.h"

const uint32_t OBJ_FILE_FORMAT_VERSION = 4;

void make_flat_shaded(std::vector<float>& vertices,
	std::vector<float>& normals,
	std::vector<float>& colors,
	std::vector<float>& tex_coords,
	std::vector<tinyobj::index_t>& indices,
	std::vector<int> face_material_indices,
	Mesh_Object_t& target);

void make_smooth_shaded(std::vector<float>& vertices,
	std::vector<float>& normals,
	std::vector<float>& colors,
	std::vector<float>& tex_coords,
	std::vector<tinyobj::index_t>& indices,
	std::vector<int> face_material_indices,
	Mesh_Object_t& target);

bool loadObjMesh(std::string& model_path, std::string& model_name, Mesh_Object_t& target, const ShadingType shadingType);
//bool loadObjMesh(std::string& full_model_path, Mesh_Object_t& target, const ShadingType shadingType);

bool readObjectFromFile(std::string file_name, Mesh_Object_t& target);
bool saveToFile(std::string file_name, Mesh_Object_t &object);