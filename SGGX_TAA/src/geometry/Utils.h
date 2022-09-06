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
#include "Octree.h"
// #include "../Renderer/Material.h"

// #define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
//#include "../3rd_party/obj/tiny_obj_loader.h"

#include "../utils/tiny_obj_utils.h"

#include "../3rd_party/std_image/std_image.h"

const uint32_t OBJ_FILE_FORMAT_VERSION = 4;

typedef struct {
	glm::vec3 center;
	glm::vec3 extents;
} AABB;

typedef struct {
	glm::vec3 v0;
	glm::vec3 v1;
	glm::vec3 v2;
} Triangle;

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

bool build_Obj_Octree(Mesh_Object_t& source, Octree &tree, size_t max_depth);

bool loadObjMesh(std::string& model_path, std::string& model_name, Mesh_Object_t& target, const ShadingType shadingType);
//bool loadObjMesh(std::string& full_model_path, Mesh_Object_t& target, const ShadingType shadingType);

bool readObjectFromFile(std::string file_name, Mesh_Object_t& target);
bool saveToFile(std::string file_name, Mesh_Object_t &object);

bool tesselateTriforce(Mesh_Object_t& object, float max_edge_length, int max_iteration);

bool calculateClampedBarycentricCoordinates(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 p, float& u, float& v, float& w);

// taken from https://www.reddit.com/r/GraphicsProgramming/comments/ko4jqu/my_taa_tutorial/ (26.08.22)
float createHaltonSequence(size_t index, int base);
/*
 * implements triangle box intersection test
 * based on https://gdbooks.gitbooks.io/3dcollisions/content/Chapter4/aabb-triangle.html
 */
bool triangleBoxIntersection(AABB box, Triangle tri);

/*
 * projects the box and the triangle onto the given axis and returns if they overlap
 */
bool axisIntersection(AABB box, Triangle tri, glm::vec3 axis);

template<typename T>
inline T max3(T value1, T value2, T value3)
{
	return glm::max(value1, glm::max(value2, value3));
}

template<typename T>
inline T min3(T value1, T value2, T value3)
{
	return glm::min(value1, glm::min(value2, value3));
}

inline float clamp01(float t) {
	if (t < 0) return 0;
	if (t > 1) return 1;
	return t;
}