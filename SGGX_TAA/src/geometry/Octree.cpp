#include "Octree.h"

#include <glm/gtc/type_ptr.hpp>

#include "../Parameters.h"

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

bool Octree::build(size_t max_depth, size_t maxPointsPerLeaf) {
	uint32_t root_index = 0;
	bool result = splitNode(0, m_lower, m_higher, maxPointsPerLeaf, 0, max_depth);
	if (result) {
		m_max_depth = max_depth;
		return true;
	}
	return false;
}

bool Octree::createSGGX()
{
	sggx_leaf_node leaf; //Leaf to dereference so program doesn't crash
	bool res = convertNode(0, &leaf);
	float max = 0;
	float mean = 0;
	for (auto& entry : renderable_leaves) {
		max = glm::max(max, entry.density);
		mean += entry.density;
	}
	mean /= renderable_leaves.size();
	return res;
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
	if (current_leaf.points.size() % 3 != 0) {
		std::cout << "Error: vertices vector size not divisible by 3" << std::endl;
		return false;
	}

	for (size_t i = 0; i < current_leaf.points.size(); i += 3) {
		glm::vec3 vec = glm::make_vec3(current_leaf.points.data() + i);
		uint32_t inner_node_index = getInnerNodeIndex(lower, higher, vec);
		new_leafs[inner_node_index].points.push_back(vec.x);
		new_leafs[inner_node_index].points.push_back(vec.y);
		new_leafs[inner_node_index].points.push_back(vec.z);
	}

	new_inner.node_indices[0] = nodes.size();
	octree_node new_node = { MAKE_LEAF(index), node_index };
	new_node.parent_index = node_index;
	nodes.push_back(new_node);
	leaves[index] = new_leafs[0];
	for (size_t i = 1; i < 8; i++) {
		new_inner.node_indices[i] = nodes.size();
		octree_node new_node = { MAKE_LEAF(leaves.size()), node_index };
		new_node.parent_index = node_index;
		nodes.push_back(new_node);
		leaves.push_back(new_leafs[i]);
	}

	inner_nodes.push_back(new_inner);

	bool result = true;
	for (size_t i = 0; i < 8; i++) {
		if (new_leafs[i].points.size() > maxPointsPerLeaf * 3 && depth + 1 < max_depth)
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

bool Octree::convertNode(uint32_t node_index, sggx_leaf_node* value)
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
		sggx_leaf_node sggx_leaf = { leaf.points.size() };
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
	sggx_leaf_node child_values[8];
	for (size_t i = 0; i < 8; i++) {
		uint32_t child_node = inner.node_indices[i];
		bool success = convertNode(child_node, &child_values[i]);
		if (!success) {
			return false;
		}
		sum += child_values[i].density;
	}

	// TODO combine child_values
	sum /= 8;
	sggx_leaf_node sggx_leaf = { sum };

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
