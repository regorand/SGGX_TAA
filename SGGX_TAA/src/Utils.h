#pragma once

#include <iostream>
#include <unordered_map>
#include "glm/glm.hpp"
#include "Renderer/Material.h"

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "3rd_party/obj/tiny_obj_loader.h"

#include "3rd_party/std_image/std_image.h"

typedef struct Mesh_Object_s {
	std::vector<float> vertices;
	std::vector<float> tex_coords;
	std::vector<float> normals;
	std::vector<float> colors;
	std::vector<unsigned int> indices;

	std::vector<Material> materials;
} Mesh_Object_t;

enum class ShadingType { SMOOTH, FLAT };

void make_flat_shaded(std::vector<float>& vertices,
	std::vector<float>& normals,
	std::vector<float>& colors,
	std::vector<float>& tex_coords,
	std::vector<tinyobj::index_t>& indices,
	Mesh_Object_t& target);

void make_smooth_shaded(std::vector<float>& vertices,
	std::vector<float>& normals,
	std::vector<float>& colors,
	std::vector<float>& tex_coords,
	std::vector<tinyobj::index_t>& indices,
	Mesh_Object_t& target);

bool loadObjMesh(std::string& model_path, std::string& model_name, Mesh_Object_t& target, const ShadingType shadingType);