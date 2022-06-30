#pragma once

#include <iostream>
#include <glm/glm.hpp>

#include "mesh_object.h"

/*
 * Structure: for x, y, z coord 0 represents the lower value, 1 the higher
 * Each Index counts up in binary using z in 1s column, y in 2s column and x in 4s column
 * 
 * So child_101_index will have the index of the child node with higher x and z coordinate and lower y coordinate
 */ 
typedef struct {
	uint32_t child_indices[8];
} inner_node_t;


typedef struct {
	uint8_t sigma_x;
	uint8_t sigma_y;
	uint8_t sigma_z;
	uint8_t r_xy;
	uint8_t r_xz;
	uint8_t r_yz;
} sggx_phase_func_t;

typedef struct {
	uint16_t location[3];
	sggx_phase_func_t phase_func;
} sggx_localised_phase_func;

#define TYPE_EXTRACT(type_and_index) ((type_and_index && 0xC0000000) >> 30)
#define INDEX_EXTRACT(type_and_index) (type_and_index && 0x3FFFFFFF)
#define TYPE_WRITE(type) (type << 30)
#define EMPTY_NODE 0
#define INNER_NODE 1
#define PHASE_NODE 2
#define LOC_PHASE_NODE 3
/*
 * Highest 2 bytes of type_and_index describe type of this node:
 * 00 empty node
 * 01 inner_node
 * 10 phase_func
 * 11 localised_phase_func
 * 
 * remaining 30 bytes are index into respective array 
 */
typedef struct {
	uint32_t type_and_index;
} octree_node_t;


class SGGX_Distribution
{
private:
	std::vector<octree_node_t> nodes;
	std::vector<inner_node_t> inner_nodes;
	std::vector<sggx_phase_func_t> phase_functions;
	std::vector<sggx_localised_phase_func> localised_phase_functions;

	uint16_t dimension;

public:
	SGGX_Distribution(uint16_t dimension, Mesh_Object_t &obj);

private:
	void scanline_triangle(Mesh_Object_t &obj, uint32_t triange_index);
	void addPhaseFunction(sggx_phase_func_t phase_func, glm::u16vec3 location);

	void splitNode(octree_node_t &node);
	uint16_t getNodeSize(glm::u16vec3 lower, glm::u16vec3 upper);

	uint8_t getSubNodeIndex(glm::u16vec3 lower, glm::u16vec3 upper, glm::u16vec3 position);
	void updateLowerUpperBounds(glm::u16vec3& lower, glm::u16vec3& upper, uint8_t sub_node_index);

	/*
	 * clamps negativ values and 0 to 0, positive values to 1
	 */
	unsigned int clamp_0_1(int val);
};

