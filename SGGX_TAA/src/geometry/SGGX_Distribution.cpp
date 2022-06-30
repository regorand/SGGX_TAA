#include "SGGX_Distribution.h"

SGGX_Distribution::SGGX_Distribution(uint16_t dimension, Mesh_Object_t &obj)
	:dimension(dimension)
{
	octree_node_t root = { 0 };
	octree_node_t empty_node = { 0 };

	nodes.push_back(root);
	nodes.push_back(empty_node);
	// TODO calculate min and max vectors
	// Calculate resolution of min grid size

}

void SGGX_Distribution::scanline_triangle(Mesh_Object_t &obj, uint32_t triangle_start_index)
{
	uint32_t idx = obj.indices[triangle_start_index];
	glm::vec3 v1 = glm::vec3(obj.vertices[3 * idx], obj.vertices[3 * idx + 1], obj.vertices[3 * idx + 2]);
	
	idx = obj.indices[triangle_start_index + 1];
	glm::vec3 v2 = glm::vec3(obj.vertices[3 * idx], obj.vertices[3 * idx + 1], obj.vertices[3 * idx + 2]);
	
	idx = obj.indices[triangle_start_index + 1];
	glm::vec3 v3 = glm::vec3(obj.vertices[3 * idx], obj.vertices[3 * idx + 1], obj.vertices[3 * idx + 2]);


}

void SGGX_Distribution::addPhaseFunction(sggx_phase_func_t phase_func, glm::u16vec3 location)
{
	glm::u16vec3 lower = glm::u16vec3(0);
	glm::u16vec3 upper = glm::u16vec3(dimension);
	uint32_t last_node_index = 0;
	octree_node_t& node = nodes[0];
	uint32_t node_type = TYPE_EXTRACT(node.type_and_index);
	uint32_t node_index = INDEX_EXTRACT(node.type_and_index);

	while (node_type == INNER_NODE) {
		uint8_t child_node_index = getSubNodeIndex(lower, upper, location);
		updateLowerUpperBounds(lower, upper, child_node_index);

		node = nodes[inner_nodes[node_index].child_indices[child_node_index]];
		node_type = TYPE_EXTRACT(node.type_and_index);
		node_index = INDEX_EXTRACT(node.type_and_index);
	}

	// TODO extract all case bodies into functions 
	switch (node_type)
	{
	case EMPTY_NODE:
	{
		uint32_t idx = localised_phase_functions.size();
		localised_phase_functions.push_back({ location.x, location.y , location.z, phase_func });
		uint32_t new_type_index = TYPE_WRITE(LOC_PHASE_NODE) + idx;
		node.type_and_index = new_type_index;
		break;
	}
	case PHASE_NODE:
	{
		std::cout << "ERROR: tried to put two phase functions into same space" << std::endl;
		break;
	}
	case LOC_PHASE_NODE:
	{
		sggx_localised_phase_func existing = localised_phase_functions[node_index];
		glm::u16vec3 existing_pos = glm::u16vec3(existing.location[0], existing.location[1], existing.location[2]);

		
		splitNode(node);
		glm::u16vec3 middle = (upper + lower) / glm::u16vec3(2);
		uint8_t existing_sub_node_index = getSubNodeIndex(lower, upper, existing_pos);
		uint8_t new_sub_node_index = getSubNodeIndex(lower, upper, location);
		while (existing_sub_node_index == new_sub_node_index) {
			updateLowerUpperBounds(lower, upper, new_sub_node_index);
			uint32_t inner_node_idx = INDEX_EXTRACT(node.type_and_index);
			node = nodes[inner_nodes[inner_node_idx].child_indices[new_sub_node_index]];

			//TODO check if node is too small ?
			splitNode(node);

			existing_sub_node_index = getSubNodeIndex(lower, upper, existing_pos);
			new_sub_node_index = getSubNodeIndex(lower, upper, location);
		}

		uint32_t inner_node_idx = INDEX_EXTRACT(node.type_and_index);

		octree_node_t& existing_sub_node = nodes[inner_nodes[inner_node_idx].child_indices[new_sub_node_index]];
		existing_sub_node.type_and_index = TYPE_WRITE(LOC_PHASE_NODE) + node_index;

		octree_node_t& new_sub_node = nodes[inner_nodes[inner_node_idx].child_indices[new_sub_node_index]];
		uint32_t new_phase_func_idx = localised_phase_functions.size();
		localised_phase_functions.push_back({ location.x, location.y , location.z, phase_func });
		new_sub_node.type_and_index = TYPE_WRITE(LOC_PHASE_NODE) + new_phase_func_idx;

		break;
	}
	default:
	{
		std::cout << "ERROR: ran into default branch of switch, should never happen" << std::endl;
		break;
	}
	}
}

void SGGX_Distribution::splitNode(octree_node_t& node)
{
	inner_node_t inner_node;

	for (int i = 0; i < 8; i++) {
		uint32_t idx = nodes.size();
		octree_node_t child = { 0 };
		nodes.push_back(child);
		inner_node.child_indices[i] = idx;
	}

	uint32_t inner_node_idx = inner_nodes.size();
	inner_nodes.push_back(inner_node);

	uint32_t new_type_index = TYPE_WRITE(LOC_PHASE_NODE) + inner_node_idx;
}

uint16_t SGGX_Distribution::getNodeSize(glm::u16vec3 lower, glm::u16vec3 upper)
{
	return glm::abs(lower.x - upper.x);
}

uint8_t SGGX_Distribution::getSubNodeIndex(glm::u16vec3 lower, glm::u16vec3 upper, glm::u16vec3 position)
{
	glm::u16vec3 middle = (upper + lower) / glm::u16vec3(2);
	glm::u16vec3 diff = position - middle;

	uint16_t diff_x = clamp_0_1(diff.x);
	uint16_t diff_y = clamp_0_1(diff.y);
	uint16_t diff_z = clamp_0_1(diff.z);

	return (diff_x << 2) + (diff_y << 1) + diff_z;
}

void SGGX_Distribution::updateLowerUpperBounds(glm::u16vec3& lower, glm::u16vec3& upper, uint8_t sub_node_index)
{
	glm::u16vec3 middle = (upper + lower) / glm::u16vec3(2);
	uint16_t diff_x = sub_node_index & 4;
	uint16_t diff_y = sub_node_index & 2;
	uint16_t diff_z = sub_node_index & 1;

	// takes the coordinates and componontwise decides if we are in the lower or upper part of the space
	lower = lower * (glm::u16vec3(1) - glm::u16vec3(diff_x, diff_y, diff_z))
		+ (middle + glm::u16vec3(1)) * glm::u16vec3(diff_x, diff_y, diff_z);

	upper = (middle + glm::u16vec3(1)) * (glm::u16vec3(1) - glm::u16vec3(diff_x, diff_y, diff_z))
		+ upper * glm::u16vec3(diff_x, diff_y, diff_z);
}

unsigned int SGGX_Distribution::clamp_0_1(int val)
{
	return glm::max(glm::sign(val), 0);
}
