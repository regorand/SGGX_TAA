#pragma once

#include <inttypes.h>
#include <vector>
#include <stack>
#include <iostream>

#include "../Renderer/RasterizationObject.h"
#include "../gl_wrappers/ShaderStorageBuffer.h"

#include <glm/glm.hpp>

#define NODE_TYPE(a) (a & 0x80000000 ? 1 : 0)
#define NODE_INDEX(a) (a & 0x7FFFFFFF)

#define MAKE_INNER(index) (index)
#define MAKE_LEAF(index) ((1 << 31) + index)

typedef struct {
	// highest level bit indicates type, 0: inner; 1: leaf
	// rest is index in corresponding array
	uint32_t type_and_index; 
	uint32_t parent_index;
	uint32_t leaf_index; //indexes into renderable_leaves array
} octree_node;

typedef struct inner_node_s {
	// indices of the child nodes in nodes vector
	// split dimension value, if lower than halfway set dimension to 0, 1 otherwise
	// index = x << 2 + y << 1 + z;
	uint32_t node_indices[8];
} inner_node;

typedef struct {
	// TODO: this should become triangle centers, and probably a struct including more data (normals, color, etc.)
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<unsigned int> indices;

	std::vector<float> points;
} obj_leaf_node;

typedef struct {
	// current shader implementation means this will have to be padded to mod 4 bytes, once we use it
	float density;
	float normal[3];
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

class Octree
{
private:
	std::vector<octree_node> nodes;
	std::vector<inner_node> inner_nodes;
	std::vector<obj_leaf_node> leaves;
	std::vector<sggx_leaf_node> renderable_leaves;

	glm::vec3 m_lower;
	glm::vec3 m_higher;

	size_t m_max_depth = 0;

	bool isInit; 
	bool buffersInit;

	std::shared_ptr<ShaderStorageBuffer> m_nodes_ssb;
	std::shared_ptr<ShaderStorageBuffer> m_inner_nodes_ssb;
	std::shared_ptr<ShaderStorageBuffer> m_leaves_ssb;

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

	bool build(size_t max_depth, size_t maxPointsPerLeaf);
	bool createSGGX();

	std::vector<obj_leaf_node> getLeaves() { return leaves; }

	bool getLeafAt(glm::vec3 position, obj_leaf_node **target);

	bool splitNode(uint32_t node_index, 
		glm::vec3 lower, 
		glm::vec3 higher, 
		size_t maxPointsPerLeaf, 
		size_t depth, 
		size_t max_depth);

	bool convertNode(uint32_t node_index, sggx_leaf_node *value, size_t current_depth);

	glm::vec3 getTreeLower();
	glm::vec3 getTreeHigher();

	bool getVisualization(octree_visualization& vis, size_t min_depth, size_t max_depth);
	std::shared_ptr<RasterizationObject> initVisBufferObject();
private:
	uint32_t getInnerNodeIndex(glm::vec3 lower, glm::vec3 higher, glm::vec3 position);
	void setNewBounds(uint32_t inner_node_index, glm::vec3* lower, glm::vec3* higher);

	bool buildNodeVisualization(octree_visualization& vis, 
		octree_node node, 
		glm::vec3 lower, 
		glm::vec3 higher, 
		size_t current_depth, 
		size_t min_depth,
		size_t max_depth);
};

