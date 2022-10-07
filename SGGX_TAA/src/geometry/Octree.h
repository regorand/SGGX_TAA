#pragma once

#include <inttypes.h>
#include <vector>
#include <stack>
#include <iostream>

#include "../Renderer/RasterizationObject.h"
#include "../gl_wrappers/ShaderStorageBuffer.h"

#include "mesh_object.h"

#include <glm/glm.hpp>

#define NODE_TYPE(a) (a & 0x80000000 ? 1 : 0)
#define NODE_INDEX(a) (a & 0x7FFFFFFF)

#define MAKE_INNER(index) (index)
#define MAKE_LEAF(index) ((1 << 31) + index)

#define INNER_NODE_TYPE 0
#define LEAF_TYPE 1

#define READ_X_VALUE_MASK(a) (a & 0x000000FF)
#define READ_Y_VALUE_MASK(a) ((a & 0x0000FF00) >> 8)
#define READ_Z_VALUE_MASK(a) ((a & 0x00FF0000) >> 16)
#define READ_SPECIAL_VALUE_MASK(a) ((a & 0xFF000000) >> 24)

#define CONVERT_X_VALUE_MASK(a) (a & 0x000000FF)
#define CONVERT_Y_VALUE_MASK(a) ((a << 8 ) & 0x0000FF00)
#define CONVERT_Z_VALUE_MASK(a) ((a << 16) & 0x00FF0000)
#define CONVERT_SPECIAL_VALUE_MASK(a) ((a << 24) & 0xFF000000)

typedef struct {
	// highest level bit indicates type, 0: inner; 1: leaf
	// rest is index in corresponding array
	uint32_t type_and_index;
	uint32_t leaf_index; //indexes into renderable_leaves array
} octree_node;

typedef struct inner_node_s {
	// indices of the child nodes in nodes vector
	// split dimension value, if lower than halfway set dimension to 0, 1 otherwise
	// index = x << 2 + y << 1 + z;
	uint32_t node_indices[8];
} inner_node;

typedef struct {
	std::vector<unsigned int> indices;
} obj_leaf_node;

typedef struct {
	// first 8 bits: x or r values
	// second 8 bits: y or g values
	// third 8 bits: z or b values
	// last 8 bits -> special


	// last 8 bits here are density;
	uint32_t sigmas;
	uint32_t rs;
	uint32_t colors;

} sggx_leaf_node;

/*
 * -----------------------------
 * Visualization
 * -----------------------------
 */

typedef struct {
	size_t first;
	size_t second;
} line_pair;

const line_pair VIS_LINE_PAIRS[12] = {
	{0, 2}, {2, 6}, {6, 4}, {0, 4},
	{4, 5}, {6, 7}, {2, 3}, {0, 1},
	{1, 5}, {5, 7}, {7, 3}, {3, 1}
};

typedef struct {
	std::vector<float> vertices;
	std::vector<unsigned int> vertex_depths;
	std::vector<unsigned int> indices;
} octree_visualization;

typedef struct texture_m {
	std::string path;
	uint8_t* buffer;
	int width;
	int height;
	int bytesPerPixel;
} Texture_s;

typedef struct {
	glm::vec3 lower;
	glm::vec3 higher;
	size_t maxPointsPerLeaf;
	size_t depth;
	size_t max_depth;
} TreeBuildParams;

class Octree
{
private:
	std::vector<octree_node> nodes;
	std::vector<inner_node> inner_nodes;
	std::vector<obj_leaf_node> leaves;
	std::vector<sggx_leaf_node> renderable_leaves;

	glm::vec3 m_lower;
	glm::vec3 m_higher;

	std::vector<float> m_tree_vertices;
	std::vector<float> m_tree_normals;
	std::vector<float> m_tree_texcoords;

	size_t m_max_depth = 0;

	bool isInit;
	bool buffersInit;

	std::shared_ptr<ShaderStorageBuffer> m_nodes_ssb;
	std::shared_ptr<ShaderStorageBuffer> m_inner_nodes_ssb;
	std::shared_ptr<ShaderStorageBuffer> m_leaves_ssb;

	std::vector<Texture_s> m_textures;

public:
	Octree();

	/* reinitiliazes octree with root node that is given leaf */
	bool init(glm::vec3 lower, glm::vec3 higher);

	bool isInitiliazed();

	// TODO maybe split up into renderable octree and renderable visualization
	bool buffersInitialized();
	bool initBuffers();
	bool bindBuffers();
	bool unbindBuffers();
	bool unInitBuffers();

	//TODO maybe create offset struct ? Or write these directly into ssbo ?
	size_t getInnerNodesOffset();
	size_t getLeavesOffset();

	size_t getNodesSize();
	size_t getInnerSize();
	size_t getLeavesSize();

	size_t getMaxDepth();


	bool buildSGGXTree(size_t max_depth, size_t maxPointsPerLeaf, Mesh_Object_t& mesh_object);
	bool buildSGGXNode(uint32_t node_index, std::vector<unsigned int> &indices, TreeBuildParams params, sggx_leaf_node& result_sggx_leaf);

	std::vector<obj_leaf_node> getLeaves() { return leaves; }

	bool getLeafAt(glm::vec3 position, obj_leaf_node** target);

	
	bool build(size_t max_depth, size_t maxPointsPerLeaf, Mesh_Object_t& mesh_object);
	bool createSGGX();
	bool splitNode(uint32_t node_index,
		glm::vec3 lower,
		glm::vec3 higher,
		size_t maxPointsPerLeaf,
		size_t depth,
		size_t max_depth);

	bool convertNode(uint32_t node_index, sggx_leaf_node* value, size_t current_depth);
	

	glm::vec3 getTreeLower();
	glm::vec3 getTreeHigher();

	bool getVisualization(octree_visualization& vis, size_t min_depth, size_t max_depth);
	std::shared_ptr<RasterizationObject> initVisBufferObject();
private:
	uint32_t getInnerNodeIndex(glm::vec3 lower, glm::vec3 higher, glm::vec3 position);
	void setNewBounds(uint32_t inner_node_index, glm::vec3* lower, glm::vec3* higher);

	bool cond_tesselate_tri(float max_edge_length,
		unsigned int index,
		std::vector<unsigned int>& in_indices,
		std::vector<unsigned int>& out_indices,
		std::vector<float>& in_vertices,
		std::vector<float>& out_vertices,
		std::vector<float>& in_normals,
		std::vector<float>& out_normals);

	glm::mat3 buildSGGXMatrix(glm::vec3 normal);
	glm::mat3 buildSGGXMatrix(sggx_leaf_node node);

	void writeSGGXMatrix(glm::mat3 matrix, sggx_leaf_node& node);


	bool buildNodeVisualization(octree_visualization& vis,
		octree_node node,
		glm::vec3 lower,
		glm::vec3 higher,
		size_t current_depth,
		size_t min_depth,
		size_t max_depth);

	glm::vec3 sampleTexture(Texture_s tex, glm::vec2 uv);
};

