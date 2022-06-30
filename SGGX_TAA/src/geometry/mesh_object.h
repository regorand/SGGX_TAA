#pragma once

#include <vector>
#include "../Renderer/Material.h"

enum class ShadingType { SMOOTH, FLAT };

typedef struct Mesh_Object_s {
	std::vector<float> vertices;
	std::vector<float> tex_coords;
	std::vector<float> normals;
	std::vector<float> colors;
	std::vector<unsigned int> indices;

	glm::vec3 lower;
	glm::vec3 higher;

	std::vector<Material> materials;

	ShadingType shadingType = ShadingType::FLAT;
} Mesh_Object_t;