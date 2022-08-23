#include "Octree.h"

#include <glm/gtc/type_ptr.hpp>

#include "../Parameters.h"
#include "Utils.h"

Octree::Octree()
	:m_lower(glm::vec3(0)), m_higher(glm::vec3(1)), isInit(false), buffersInit(false)
{

}

bool Octree::isInitiliazed()
{
	return isInit;
}

bool Octree::buffersInitialized()
{
	return buffersInit;
}

bool Octree::initBuffers()
{
	const unsigned int nodes_target = 1;
	const unsigned int inner_nodes_target = 2;
	const unsigned int leaves_target = 3;

	m_nodes_ssb = std::make_shared<ShaderStorageBuffer>(nodes.data(), nodes.size() * sizeof(octree_node), nodes_target);
	m_inner_nodes_ssb
		= std::make_shared<ShaderStorageBuffer>(inner_nodes.data(), inner_nodes.size() * sizeof(inner_node), inner_nodes_target);
	m_leaves_ssb
		= std::make_shared<ShaderStorageBuffer>(renderable_leaves.data(), renderable_leaves.size() * sizeof(sggx_leaf_node), leaves_target);

	/*
	size_t nodes_byte_size = nodes.size() * sizeof(octree_node);
	size_t inner_nodes_byte_size = inner_nodes.size() * sizeof(inner_node);
	size_t leaves_size = renderable_leaves.size() * sizeof(sggx_leaf_node);

	size_t leaves_offset = inner_nodes_byte_size + nodes_byte_size;

	// Need to create proper, const sized leaves before pushing them to the buffer
	size_t total_size = nodes_byte_size+ inner_nodes_byte_size + leaves_size;

	unsigned char* combined_buffer = new unsigned char[total_size];

	memcpy(combined_buffer, nodes.data(), nodes_byte_size);
	memcpy(combined_buffer + nodes_byte_size, inner_nodes.data(), inner_nodes_byte_size);
	memcpy(combined_buffer + leaves_offset, renderable_leaves.data(), leaves_size);

	m_octree_ssb = std::make_shared<ShaderStorageBuffer>(combined_buffer, total_size, target);
	delete[] combined_buffer;
	*/

	buffersInit = true;
	return true;
}

bool Octree::bindBuffers()
{
	if (m_nodes_ssb == nullptr) {
		return false;
	}

	if (m_inner_nodes_ssb == nullptr) {
		return false;
	}

	if (m_leaves_ssb == nullptr) {
		return false;
	}
	m_inner_nodes_ssb->bind();
	m_nodes_ssb->bind();
	m_leaves_ssb->bind();
	return true;
}

bool Octree::unbindBuffers()
{
	bool anyFail = false;
	if (m_nodes_ssb != nullptr) {
		m_nodes_ssb->unbind();
	}
	else anyFail = true;

	if (m_inner_nodes_ssb != nullptr) {
		m_inner_nodes_ssb->unbind();
	}
	else anyFail = true;

	if (m_leaves_ssb != nullptr) {
		m_leaves_ssb->unbind();
	}
	else anyFail = true;

	return !anyFail;
}

bool Octree::unInitBuffers()
{
	m_nodes_ssb = nullptr;
	m_inner_nodes_ssb = nullptr;
	m_leaves_ssb = nullptr;
	buffersInit = false;
	return true;
}

bool Octree::init(glm::vec3 lower, glm::vec3 higher)
{
	if (nodes.size() > 0) {
		nodes = std::vector<octree_node>();
	}

	if (inner_nodes.size() > 0) {
		inner_nodes = std::vector<inner_node>();
	}

	if (leaves.size() > 0) {
		leaves = std::vector<obj_leaf_node>();
	}
	glm::vec3 diff = higher - lower;
	float max_dimension = glm::max(diff.x, glm::max(diff.y, diff.z));
	m_lower = lower;
	m_higher = m_lower + glm::vec3(max_dimension);

	uint32_t index = 0;
	uint32_t type = 1;

	uint32_t type_and_index = index + (type << 31);

	nodes.push_back({ type_and_index });
	leaves.push_back({});
	isInit = true;
	return true;
}

size_t Octree::getInnerNodesOffset()
{
	return nodes.size() * sizeof(octree_node);
}

size_t Octree::getLeavesOffset()
{
	return nodes.size() * sizeof(octree_node) + inner_nodes.size() * sizeof(inner_node);
}

size_t Octree::getNodesSize()
{
	return nodes.size();
}

size_t Octree::getInnerSize()
{
	return inner_nodes.size();
}

size_t Octree::getLeavesSize()
{
	return renderable_leaves.size();
}

size_t Octree::getMaxDepth()
{
	return m_max_depth;
}

bool Octree::build(size_t max_depth, size_t maxPointsPerLeaf, Mesh_Object_t& mesh_object) {
	if (nodes.empty()) return false;

	uint32_t root_index = 0;

	octree_node current = nodes[0];

	uint32_t type = NODE_TYPE(current.type_and_index);
	uint32_t index = NODE_INDEX(current.type_and_index);

	if (type != LEAF_TYPE) return false;


	m_tree_vertices = mesh_object.vertices;
	m_tree_normals = mesh_object.normals;


	leaves[index].indices = mesh_object.indices;


	bool result = splitNode(0, m_lower, m_higher, maxPointsPerLeaf, 0, max_depth);

	m_tree_vertices.clear();
	m_tree_normals.clear();

	if (result) {
		m_max_depth = max_depth;
		return true;
	}
	return false;
}

bool Octree::createSGGX()
{
	sggx_leaf_node leaf; //Leaf to dereference so program doesn't crash
	bool res = convertNode(0, &leaf, 0);
	float max = 0;
	float mean = 0;
	for (auto& entry : renderable_leaves) {
		max = glm::max(max, entry.density);
		mean += entry.density;
	}
	mean /= renderable_leaves.size();
	return res;
}

bool Octree::buildSGGXTree(size_t max_depth, size_t maxPointsPerLeaf, Mesh_Object_t& mesh_object)
{
	if (mesh_object.vertices.size() % 3 != 0) {
		std::cout << "Error, invalid size for vertices Array" << std::endl;
		return false;
	}

	// create single leaf node as root
	init(mesh_object.lower, mesh_object.higher);

	if (nodes.empty()) return false;

	uint32_t root_index = 0;

	octree_node current = nodes[0];

	uint32_t type = NODE_TYPE(current.type_and_index);
	uint32_t index = NODE_INDEX(current.type_and_index);

	if (type != LEAF_TYPE) return false;

	m_tree_vertices = mesh_object.vertices;
	m_tree_normals = mesh_object.normals;

	leaves[index].indices = mesh_object.indices;

	TreeBuildParams params = { m_lower, m_higher, maxPointsPerLeaf, 0, max_depth };
	sggx_leaf_node dummy_leaf;

	std::vector<unsigned int> indices_copy = mesh_object.indices;

	bool result = buildSGGXNode(0, indices_copy, params, dummy_leaf);

	m_tree_vertices.clear();
	m_tree_normals.clear();

	if (result) {
		m_max_depth = max_depth;
		return true;
	}
	return false;
}

bool Octree::buildSGGXNode(uint32_t node_index, std::vector<unsigned int>& indices, TreeBuildParams params, sggx_leaf_node& result_sggx_leaf)
{
	if (node_index >= nodes.size()) {
		std::cout << "Error: Index error on octree node" << std::endl;
		return false;
	}

	octree_node node = nodes[node_index];
	uint32_t type = NODE_TYPE(node.type_and_index);
	uint32_t index = NODE_INDEX(node.type_and_index);

	if (!type) {
		return true;
	}

	// 1. if at max depth or empty -> goto 6.
	if (params.depth < params.max_depth && indices.size() > 0) {
		inner_node new_inner;
		node.type_and_index = MAKE_INNER(inner_nodes.size());

		new_inner.node_indices[0] = nodes.size();
		octree_node new_node = { MAKE_LEAF(index) };
		nodes.push_back(new_node);
		for (size_t i = 1; i < 8; i++) {
			new_inner.node_indices[i] = nodes.size();
			octree_node new_node = { MAKE_LEAF(leaves.size()) };

			nodes.push_back(new_node);
		}

		inner_nodes.push_back(new_inner);

		// aggregation values over all children
		float sum = 0;
		glm::vec3 normal = glm::vec3(0);
		glm::mat3 sggx_mat = glm::mat3(0);

		std::vector<unsigned int> child_indices;
		bool result = true;
		if (indices.size() % 3 == 0) {
			for (size_t i = 0; i < 8; i++) {
				child_indices.clear();
				glm::vec3 new_lower = params.lower;
				glm::vec3 new_higher = params.higher;
				setNewBounds(i, &new_lower, &new_higher);

				glm::vec3 box_center = (new_lower + new_higher) / glm::vec3(2);
				glm::vec3 extents = box_center - new_lower;

				for (size_t j = 0; j < indices.size(); j += 3) {
					glm::vec3 v1 = glm::make_vec3(m_tree_vertices.data() + 3 * indices[j]);
					glm::vec3 v2 = glm::make_vec3(m_tree_vertices.data() + 3 * indices[j + 1]);
					glm::vec3 v3 = glm::make_vec3(m_tree_vertices.data() + 3 * indices[j + 2]);

					if (triangleBoxIntersection({ box_center, extents }, { v1, v2, v3 })) {
						child_indices.push_back(indices[j]);
						child_indices.push_back(indices[j + 1]);
						child_indices.push_back(indices[j + 2]);
					}
				}

				sggx_leaf_node child_value;
				TreeBuildParams new_params = { new_lower, new_higher, params.maxPointsPerLeaf, params.depth + 1, params.max_depth };

				result &= buildSGGXNode(new_inner.node_indices[i],
					child_indices,
					new_params,
					child_value);

				sum += child_value.density;
				sggx_mat += buildSGGXMatrix(child_value);
			}
			std::vector<unsigned int>().swap(indices);
			std::vector<unsigned int>().swap(child_indices);

			if (!result) return false;

			sum /= 8;
			if (sum < 0) {
				int a = 0;
			}
			glm::mat3 downscaled;
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					downscaled[i][j] = sggx_mat[i][j] / 8;
				}
			}
			result_sggx_leaf = { sum };
			writeSGGXMatrix(downscaled, result_sggx_leaf);
		}
	}
	else {
		size_t depth_diff = m_max_depth - params.depth;
		float size_factor = 1.0f / (glm::pow(2.0f, depth_diff));

		glm::vec3 normal = glm::vec3(0);
		size_t count = 0;

		for (size_t i = 0; i < indices.size(); i+=3) {
			// TODO here:
			// get normals for triangle in total and each vertex
			glm::vec3 v1 = glm::make_vec3(m_tree_vertices.data() + 3 * indices[i]);
			glm::vec3 v2 = glm::make_vec3(m_tree_vertices.data() + 3 * indices[i + 1]);
			glm::vec3 v3 = glm::make_vec3(m_tree_vertices.data() + 3 * indices[i + 2]);

			glm::vec3 triangle_plane_normal = glm::normalize(glm::cross(glm::normalize(v2 - v1), glm::normalize(v3 - v1)));

			glm::vec3 n1 = glm::make_vec3(m_tree_normals.data() + 3 * indices[i]);
			glm::vec3 n2 = glm::make_vec3(m_tree_normals.data() + 3 * indices[i + 1]);
			glm::vec3 n3 = glm::make_vec3(m_tree_normals.data() + 3 * indices[i + 2]);

			// project voxel center along normal (shortest distance) onto triangle
			glm::vec3 voxel_center = (params.lower + params.higher) / glm::vec3(2);

			glm::vec3 dist_vector = voxel_center - v1;
			float dist = glm::dot(dist_vector, triangle_plane_normal);

			glm::vec3 projected_point = voxel_center - dist * triangle_plane_normal;
			// take that position and interpolate normals to that position

			float u, v, w;
			calculateClampedBarycentricCoordinates(v1, v2, v3, projected_point, u, v, w);

			normal += u * n1 + v * n2 + w * n3;

			// idea only take one normal ?

			// normal += glm::make_vec3(m_tree_normals.data() + 3 * indices[i]);
			count++;
		}

		result_sggx_leaf = { 0 };
		if (count != 0) {
			normal = glm::normalize(normal);

			glm::mat3 SGGX_mat = buildSGGXMatrix(normal);
			writeSGGXMatrix(SGGX_mat, result_sggx_leaf);
		}

		// result_sggx_leaf.normal[0] = normal.x;
		// result_sggx_leaf.normal[1] = normal.y;
		// result_sggx_leaf.normal[2] = normal.z;

		result_sggx_leaf.density = 0;
		if (indices.size() > 0) {
			result_sggx_leaf.density = 1;
		}
	}

	size_t sggx_leaf_index = renderable_leaves.size();
	node.leaf_index = sggx_leaf_index;

	renderable_leaves.push_back(result_sggx_leaf);
	nodes[node_index] = node;

	return true;
}

bool Octree::getLeafAt(glm::vec3 position, obj_leaf_node** target)
{
	octree_node current = nodes[0];
	uint32_t type = NODE_TYPE(current.type_and_index);
	uint32_t index = NODE_INDEX(current.type_and_index);

	glm::vec3 lower = m_lower;
	glm::vec3 higher = m_higher;

	while (!type) {
		uint32_t inner_node_index = getInnerNodeIndex(lower, higher, position);

		if (index < inner_nodes.size()) {
			inner_node inner = inner_nodes[index];
			uint32_t child_node_index = inner.node_indices[inner_node_index];
			if (child_node_index < nodes.size()) {
				setNewBounds(inner_node_index, &lower, &higher);

				current = nodes[child_node_index];
				type = NODE_TYPE(current.type_and_index);
				index = NODE_INDEX(current.type_and_index);
			}
			else {
				std::cout << "Error, invalid octree node index" << std::endl;
				return false;
			}
		}
		else {
			std::cout << "Error, invalid inner node index" << std::endl;
			return false;
		}
	}

	if (index < leaves.size()) {
		*target = &leaves[index];
		return true;
	}
	std::cout << "Error, invalid leaf node index" << std::endl;
	return false;
}

bool Octree::splitNode(uint32_t node_index, glm::vec3 lower, glm::vec3 higher, size_t maxPointsPerLeaf, size_t depth, size_t max_depth)
{
	if (node_index >= nodes.size()) {
		std::cout << "Error: Index error on octree node" << std::endl;
		return false;
	}

	octree_node& node = nodes[node_index];
	uint32_t type = NODE_TYPE(node.type_and_index);
	uint32_t index = NODE_INDEX(node.type_and_index);

	if (!type) {
		// TODO: figure out what happens when trying to split inner nodes 
		// -> Just send split call down the hierarchy ?
		// case should never really happen here though, Tree is always built from scratch
		return true;
	}

	obj_leaf_node current_leaf = leaves[index];

	if (depth >= max_depth) {
		return true;
	}



	if (index >= leaves.size()) {
		std::cout << "Error: Index error on octree leaves" << std::endl;
		return false;
	}

	inner_node new_inner;
	node.type_and_index = MAKE_INNER(inner_nodes.size());


	obj_leaf_node new_leafs[8];

	if (current_leaf.indices.size() % 3 == 0) {

		std::vector<float> local_vertices;
		std::vector<float> local_normals;
		std::vector<unsigned int> local_indices;
		for (size_t i = 0; i < current_leaf.indices.size(); i += 3) {

			// tesselate_triangle(

		// TODO tesselate here if not at max_depth ?
		// extra parameter for min_leaf_depth ?
		// tesselate to match children size


			glm::vec3 v1 = glm::make_vec3(m_tree_vertices.data() + 3 * current_leaf.indices[i]);
			glm::vec3 v2 = glm::make_vec3(m_tree_vertices.data() + 3 * current_leaf.indices[i + 1]);
			glm::vec3 v3 = glm::make_vec3(m_tree_vertices.data() + 3 * current_leaf.indices[i + 2]);

			for (size_t j = 0; j < 8; j++) {
				glm::vec3 new_lower = lower;
				glm::vec3 new_higher = higher;
				setNewBounds(j, &new_lower, &new_higher);

				glm::vec3 box_center = (new_lower + new_higher) / glm::vec3(2);
				glm::vec3 extents = box_center - new_lower;

				if (triangleBoxIntersection({ box_center, extents }, { v1, v2, v3 })) {
					new_leafs[j].indices.push_back(current_leaf.indices[i]);
					new_leafs[j].indices.push_back(current_leaf.indices[i + 1]);
					new_leafs[j].indices.push_back(current_leaf.indices[i + 2]);
				}
			}

			/*
			glm::vec3 v1 = glm::make_vec3(m_tree_vertices.data() + 3 * current_leaf.indices[i]);
			glm::vec3 v2 = glm::make_vec3(m_tree_vertices.data() + 3 * current_leaf.indices[i + 1]);
			glm::vec3 v3 = glm::make_vec3(m_tree_vertices.data() + 3 * current_leaf.indices[i + 2]);


			glm::vec3 n1 = glm::make_vec3(m_tree_normals.data() + 3 * current_leaf.indices[i]);
			glm::vec3 n2 = glm::make_vec3(m_tree_normals.data() + 3 * current_leaf.indices[i + 1]);
			glm::vec3 n3 = glm::make_vec3(m_tree_normals.data() + 3 * current_leaf.indices[i + 2]);

			// use vertices to get triangle center
			glm::vec3 center = (v1 + v2 + v3) / glm::vec3(3);

			// use triangle center to get inner_index
			uint32_t inner_node_index = getInnerNodeIndex(lower, higher, center);


			// push new index
			new_leafs[inner_node_index].indices.push_back(current_leaf.indices[i]);
			new_leafs[inner_node_index].indices.push_back(current_leaf.indices[i + 1]);
			new_leafs[inner_node_index].indices.push_back(current_leaf.indices[i + 2]);

			*/

		}
	}

	// current_leaf.indices.clear();

	new_inner.node_indices[0] = nodes.size();
	octree_node new_node = { MAKE_LEAF(index) };
	nodes.push_back(new_node);
	leaves[index] = new_leafs[0];
	for (size_t i = 1; i < 8; i++) {
		new_inner.node_indices[i] = nodes.size();
		octree_node new_node = { MAKE_LEAF(leaves.size()) };
		if (leaves.size() == 4650589) {
			int a = 0;
		}
		nodes.push_back(new_node);
		leaves.push_back(new_leafs[i]);
	}

	inner_nodes.push_back(new_inner);

	bool result = true;
	for (size_t i = 0; i < 8; i++) {
		if (new_leafs[i].indices.size() > 0 && depth + 1 < max_depth)
		{
			glm::vec3 new_lower = lower;
			glm::vec3 new_higher = higher;
			setNewBounds(i, &new_lower, &new_higher);
			result &= splitNode(new_inner.node_indices[i],
				new_lower,
				new_higher,
				maxPointsPerLeaf,
				depth + 1,
				max_depth);
		}
		else {
			/*
			node.leaf_index = renderable_leaves.size();
			renderable_leaves.push_back({ new_leafs[i].points.size() > 0 ? 1.0f : 0.0f });
			return true;
			*/
		}
	}

	return result;
}

bool Octree::convertNode(uint32_t node_index, sggx_leaf_node* value, size_t current_depth)
{
	if (node_index >= nodes.size()) {
		std::cout << "Error: Index error on octree node" << std::endl;
		return false;
	}

	octree_node& node = nodes[node_index];
	uint32_t type = NODE_TYPE(node.type_and_index);
	uint32_t index = NODE_INDEX(node.type_and_index);

	if (type) {
		if (index >= leaves.size()) {
			std::cout << "Error: Index error on octree leaves" << std::endl;
			return false;
		}
		obj_leaf_node leaf = leaves[index];
		// TODO create function convertLeafToSGGX() or similar

		size_t depth_diff = m_max_depth - current_depth;

		// Try out which makes more sense 
		// size_factor scaling with volume makes more sense for voxels
		// size_facor scaling with length should give roughly same results for leaves that are at a low depth, when
		// rendering using intersection length

		float size_factor = 1.0f / (glm::pow(2.0f, depth_diff));
		//float size_factor = 1.0f / (glm::pow(8.0f, depth_diff));

		glm::vec3 normal = glm::vec3(0);
		size_t count = 0;

		for (size_t i = 0; i < leaf.indices.size(); i++) {
			normal += glm::make_vec3(m_tree_normals.data() + 3 * leaf.indices[i]);
			count++;
		}
		sggx_leaf_node sggx_leaf = { 0 };
		if (count != 0) {
			normal = glm::normalize(normal);

			glm::mat3 SGGX_mat = buildSGGXMatrix(normal);
			writeSGGXMatrix(SGGX_mat, sggx_leaf);
		}

		// sggx_leaf.normal[0] = normal.x;
		// sggx_leaf.normal[1] = normal.y;
		// sggx_leaf.normal[2] = normal.z;

		sggx_leaf.density = 0;
		if (leaf.indices.size() > 0) {
			sggx_leaf.density = size_factor;
		}
		node.leaf_index = renderable_leaves.size();
		renderable_leaves.push_back(sggx_leaf);
		*value = sggx_leaf;
		return true;
	}

	if (index >= inner_nodes.size()) {
		std::cout << "Error: Index error on octree inner nodes" << std::endl;
		return false;
	}

	inner_node inner = inner_nodes[index];

	float sum = 0;
	glm::vec3 normal = glm::vec3(0);
	glm::mat3 sggx_mat = glm::mat3(0);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			sggx_mat[i][j] = 0;
		}
	}
	sggx_leaf_node child_values[8];
	for (size_t i = 0; i < 8; i++) {
		uint32_t child_node = inner.node_indices[i];
		bool success = convertNode(child_node, &child_values[i], current_depth + 1);
		if (!success) {
			return false;
		}
		sum += child_values[i].density;
		//normal += glm::make_vec3(child_values[i].normal);
		sggx_mat += buildSGGXMatrix(child_values[i]);



	}
	// TODO combine child_values
	sum /= 8;
	if (sum < 0) {
		int a = 0;
	}
	//normal = glm::normalize(normal);
	//sggx_mat /= glm::mat3(8, 8, 8, 8, 8, 8, 8, 8, 8);
	glm::mat3 downscaled;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			downscaled[i][j] = sggx_mat[i][j] / 8;
		}
	}
	sggx_leaf_node sggx_leaf = { sum };
	//sggx_leaf_node sggx_leaf = { sum, { normal.x, normal.y, normal.z } };
	writeSGGXMatrix(downscaled, sggx_leaf);

	node.leaf_index = renderable_leaves.size();
	renderable_leaves.push_back(sggx_leaf);
	*value = sggx_leaf;
	return true;
}

glm::vec3 Octree::getTreeLower()
{
	return m_lower;
}

glm::vec3 Octree::getTreeHigher()
{
	return m_higher;
}

uint32_t Octree::getInnerNodeIndex(glm::vec3 lower, glm::vec3 higher, glm::vec3 position) {
	glm::vec3 halfway = (higher + lower) / glm::vec3(2);
	bool x_fac = position.x > halfway.x;
	bool y_fac = position.y > halfway.y;
	bool z_fac = position.z > halfway.z;

	return (x_fac << 2) + (y_fac << 1) + z_fac;
}

void Octree::setNewBounds(uint32_t inner_node_index, glm::vec3* lower, glm::vec3* higher) {
	glm::vec3 halfway = (*higher + *lower) / glm::vec3(2);
	glm::vec3 factors((inner_node_index & 0x4) >> 2, (inner_node_index & 0x2) >> 1, inner_node_index & 0x1);
	glm::vec3 one_minus_factors = glm::vec3(1) - factors;
	*higher = factors * *higher + one_minus_factors * halfway;
	*lower = one_minus_factors * *lower + factors * halfway;
}

bool Octree::cond_tesselate_tri(float max_edge_length,
	unsigned int index,
	std::vector<unsigned int>& in_indices,
	std::vector<unsigned int>& out_indices,
	std::vector<float>& in_vertices,
	std::vector<float>& out_vertices,
	std::vector<float>& in_normals,
	std::vector<float>& out_normals)
{
	glm::vec3 v1 = glm::make_vec3(in_vertices.data() + 3 * in_indices[index]);
	glm::vec3 v2 = glm::make_vec3(in_vertices.data() + 3 * in_indices[index + 1]);
	glm::vec3 v3 = glm::make_vec3(in_vertices.data() + 3 * in_indices[index + 2]);


	out_vertices.push_back(v1.x);
	out_vertices.push_back(v1.y);
	out_vertices.push_back(v1.z);

	out_vertices.push_back(v2.x);
	out_vertices.push_back(v2.y);
	out_vertices.push_back(v2.z);

	out_vertices.push_back(v3.x);
	out_vertices.push_back(v3.y);
	out_vertices.push_back(v3.z);


	glm::vec3 n1 = glm::make_vec3(in_normals.data() + 3 * in_indices[index]);
	glm::vec3 n2 = glm::make_vec3(in_normals.data() + 3 * in_indices[index + 1]);
	glm::vec3 n3 = glm::make_vec3(in_normals.data() + 3 * in_indices[index + 2]);

	out_normals.push_back(n1.x);
	out_normals.push_back(n1.y);
	out_normals.push_back(n1.z);

	out_normals.push_back(n2.x);
	out_normals.push_back(n2.y);
	out_normals.push_back(n2.z);

	out_normals.push_back(n3.x);
	out_normals.push_back(n3.y);
	out_normals.push_back(n3.z);

	float l12 = glm::length(v1 - v2);
	float l23 = glm::length(v2 - v3);
	float l31 = glm::length(v3 - v1);

	float max_length = glm::max(l12, glm::max(l23, l31));

	if (max_length <= max_edge_length) {
		out_indices.push_back(0);
		out_indices.push_back(1);
		out_indices.push_back(2);

		//returns false because no tesselation
		return false;
	}

	glm::vec3 triangle_normal = (n1 + n2 + n3) / glm::vec3(3);

	for (int i = 0; i < 3; i++) {
		out_normals.push_back(triangle_normal.x);
		out_normals.push_back(triangle_normal.y);
		out_normals.push_back(triangle_normal.z);
	}

	glm::vec3 v12 = (v1 + v2) / glm::vec3(2);
	glm::vec3 v23 = (v2 + v3) / glm::vec3(2);
	glm::vec3 v31 = (v3 + v1) / glm::vec3(2);

	out_vertices.push_back(v12.x);
	out_vertices.push_back(v12.y);
	out_vertices.push_back(v12.z);

	out_vertices.push_back(v23.x);
	out_vertices.push_back(v23.y);
	out_vertices.push_back(v23.z);

	out_vertices.push_back(v31.x);
	out_vertices.push_back(v31.y);
	out_vertices.push_back(v31.z);

	out_indices.push_back(0);
	out_indices.push_back(3);
	out_indices.push_back(5);

	out_indices.push_back(3);
	out_indices.push_back(4);
	out_indices.push_back(5);

	out_indices.push_back(3);
	out_indices.push_back(1);
	out_indices.push_back(4);

	out_indices.push_back(4);
	out_indices.push_back(2);
	out_indices.push_back(5);

	return true;
}

glm::mat3 Octree::buildSGGXMatrix(glm::vec3 normal)
{
	float squared_roughness = parameters.roughness * parameters.roughness;

	float x_sq = normal.x * normal.x;
	float y_sq = normal.y * normal.y;
	float z_sq = normal.z * normal.z;

	float xy = normal.x * normal.y;
	float xz = normal.x * normal.z;
	float yz = normal.y * normal.z;

	// matrix constructor is column major
	return glm::mat3(x_sq + squared_roughness * y_sq * z_sq, xy - squared_roughness * xy, xz - squared_roughness * xz,
		xy - squared_roughness * xy, y_sq + squared_roughness * x_sq * z_sq, yz - squared_roughness * yz,
		xz - squared_roughness * xz, yz - squared_roughness * yz, z_sq + squared_roughness * x_sq * y_sq);
}

glm::mat3 Octree::buildSGGXMatrix(sggx_leaf_node node)
{

	float Sxx = node.sigma_x * node.sigma_x;
	float Syy = node.sigma_y * node.sigma_y;
	float Szz = node.sigma_z * node.sigma_z;

	float Sxy = node.r_xy * node.sigma_x * node.sigma_y;
	float Sxz = node.r_xz * node.sigma_x * node.sigma_z;
	float Syz = node.r_yz * node.sigma_y * node.sigma_z;

	return glm::mat3(Sxx, Sxy, Sxz, Sxy, Syy, Syz, Sxz, Syz, Szz);
}

void Octree::writeSGGXMatrix(glm::mat3 matrix, sggx_leaf_node& node)
{
	float Sxx = matrix[0][0];
	float Syy = matrix[1][1];
	float Szz = matrix[2][2];


	node.sigma_x = glm::sqrt(Sxx);
	node.sigma_y = glm::sqrt(Syy);
	node.sigma_z = glm::sqrt(Szz);

	node.r_xy = matrix[0][1] / glm::sqrt(Sxx * Syy);
	node.r_xz = matrix[0][2] / glm::sqrt(Sxx * Szz);
	node.r_yz = matrix[1][2] / glm::sqrt(Syy * Szz);
}

bool Octree::buildNodeVisualization(octree_visualization& vis, octree_node node, glm::vec3 lower, glm::vec3 higher, size_t current_depth, size_t min_depth, size_t max_depth)
{
	if (current_depth >= max_depth) {
		return true;
	}

	uint32_t type = NODE_TYPE(node.type_and_index);
	uint32_t index = NODE_INDEX(node.type_and_index);

	if (current_depth >= min_depth) {

		size_t local_vertex_indices[8];
		size_t vertices_offset = vis.vertices.size() / 3;

		for (size_t i = 0; i < 8; i++) {
			glm::vec3 factors((i & 0x4) >> 2, (i & 0x2) >> 1, i & 0x1);
			glm::vec3 one_minus_factors = glm::vec3(1) - factors;
			glm::vec3 vertex = one_minus_factors * lower + factors * higher;

			//TODO, use map to index into vis.vertices so we an resuse vertices
			vis.vertices.push_back(vertex.x);
			vis.vertices.push_back(vertex.y);
			vis.vertices.push_back(vertex.z);
			vis.vertex_depths.push_back(current_depth);
			local_vertex_indices[i] = i; // Later, maybe read index from map instead
		}

		for (size_t i = 0; i < 12; i++) {
			line_pair pair = VIS_LINE_PAIRS[i];
			vis.indices.push_back(vertices_offset + local_vertex_indices[pair.first]);
			vis.indices.push_back(vertices_offset + local_vertex_indices[pair.second]);
		}
	}

	if (type) return true;

	inner_node inner = inner_nodes[index];

	bool result = true;
	for (size_t i = 0; i < 8; i++) {
		glm::vec3 new_lower = lower;
		glm::vec3 new_higher = higher;
		setNewBounds(i, &new_lower, &new_higher);
		buildNodeVisualization(vis, nodes[inner.node_indices[i]], new_lower, new_higher, current_depth + 1, min_depth, max_depth);
	}

	return result;
}

bool Octree::getVisualization(octree_visualization& vis, size_t min_depth, size_t max_depth)
{
	//TODO go recurselivy through tree to build complete visualization
	// for now only visualize root node

	// maybe extract this to ots own function
	return buildNodeVisualization(vis, nodes[0], m_lower, m_higher, 0, min_depth, max_depth);
}

std::shared_ptr<RasterizationObject> Octree::initVisBufferObject()
{
	octree_visualization vis;
	getVisualization(vis, parameters.min_visualization_depth, parameters.max_visualization_depth);

	const std::string vert_path = "res/shaders/octree_vis.vert";
	const std::string frag_path = "res/shaders/octree_vis.frag";

	std::shared_ptr<Shader> shader = std::make_shared<Shader>(vert_path, frag_path);

	shader->bind();
	std::shared_ptr<VertexArray> va = std::make_shared<VertexArray>();

	std::shared_ptr<ArrayBuffer> vertices
		= std::make_shared<ArrayBuffer>(vis.vertices.data(), vis.vertices.size() * sizeof(float));
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	const size_t glsl_uint_size = sizeof(GL_UNSIGNED_INT);
	std::shared_ptr<ArrayBuffer> depths
		= std::make_shared<ArrayBuffer>(vis.vertex_depths.data(), vis.vertex_depths.size() * sizeof(unsigned int));
	glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, glsl_uint_size, 0);
	glEnableVertexAttribArray(1);

	glm::mat4 model_matrix(1); // TODO put this as a member variable

	std::shared_ptr<IndexBuffer> ib
		= std::make_shared<IndexBuffer>(vis.indices.data(), vis.indices.size());

	std::vector<Material> materials;
	std::shared_ptr<RasterizationObject> object
		= std::make_shared<RasterizationObject>(va, ib, shader, model_matrix, materials);

	object->addArrayBuffer(vertices);
	object->addArrayBuffer(depths);
	shader->unbind();

	return object;
}
