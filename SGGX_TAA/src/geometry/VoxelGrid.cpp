#include "VoxelGrid.h"

VoxelGrid::VoxelGrid(uint16_t dimension)
	:m_dimension(dimension)
{
	m_lower = glm::vec3(0);
	m_higher = glm::vec3(1);
	m_data = new voxel_t[m_dimension * m_dimension * m_dimension];
	for (unsigned int i = 0; i < m_dimension * m_dimension * m_dimension; i++) {
		m_data[i] = { 0 };
	}
	m_ssb = nullptr;
}

VoxelGrid::~VoxelGrid()
{
	delete m_data;
}

uint16_t VoxelGrid::getDimension()
{
	return m_dimension;
}

void VoxelGrid::setLower(glm::vec3 lower)
{
	m_lower = lower;
}

void VoxelGrid::setHigher(glm::vec3 higher)
{
	m_higher = higher;
}

glm::vec3 VoxelGrid::getLower()
{
	return m_lower;
}

glm::vec3 VoxelGrid::getHigher()
{
	return m_higher;
}		

float VoxelGrid::getVoxelSize()
{
	auto diff = m_higher - m_lower;
	float max_diff = glm::max(diff.x, glm::max(diff.y, diff.z));
	return max_diff / m_dimension;
}

glm::vec3 VoxelGrid::getCenterOfVoxel(glm::u16vec3 indices)
{
	float voxelSize = getVoxelSize();
	return m_lower + glm::vec3(voxelSize / 2) + glm::vec3(indices) * glm::vec3(voxelSize);
}

void VoxelGrid::setVoxel(glm::u16vec3 indices, voxel_t new_voxel)
{
	if (!checkDimension(indices)) {
		std::cout << "Voxelgrid Error: Index Error on index vector: [ " 
			<< indices.x << ", " << indices.y << ", " << indices.z << "]" << std::endl;
	}

	m_data[indices.x + indices.y * m_dimension + indices.z * m_dimension * m_dimension] = new_voxel;
}

voxel_t VoxelGrid::getVoxel(glm::u16vec3 indices)
{
	if (!checkDimension(indices)) {
		std::cout << "Voxelgrid Error: Index Error on index vector: [ "
			<< indices.x << ", " << indices.y << ", " << indices.z << "]" << std::endl;
		return voxel_t();
	}

	return m_data[indices.x + indices.y * m_dimension + indices.z * m_dimension * m_dimension];
}

voxel_t* VoxelGrid::getVoxelData()
{
	return m_data;
}

bool VoxelGrid::isInit()
{
	return m_isInit;
}

void VoxelGrid::initBuffers()
{
	const unsigned int target = 0;
	m_ssb = std::make_shared<ShaderStorageBuffer>(m_data, m_dimension * m_dimension * m_dimension * sizeof(voxel_t), target);
	m_isInit = true;
}

void VoxelGrid::deleteBuffers()
{
	m_ssb = nullptr;
	m_isInit = false;
}

bool VoxelGrid::bindBuffers()
{
	if (m_ssb != nullptr) {
		m_ssb->bind();
		return true;
	}
	return false;
}

bool VoxelGrid::unbindBuffers()
{
	if (m_ssb != nullptr) {
		m_ssb->unbind();
		return true;
	}
	return false;
}

unsigned int VoxelGrid::countVoxels()
{
	return glm::pow((uint32_t)m_dimension, 3);
}

unsigned int VoxelGrid::countNonEmptyVoxels()
{
	float eps = 1e-7;
	unsigned int count = 0;
	for (uint32_t i = 0; i < glm::pow((uint32_t)m_dimension, 3); i++) {
		if (m_data[i].val > eps) {
			count++;
		}
	}
	return count;
}

bool VoxelGrid::checkDimension(glm::u16vec3 indices)
{
	return indices.x >= 0 && indices.x < m_dimension
		&& indices.y >= 0 && indices.y < m_dimension
		&& indices.z >= 0 && indices.z < m_dimension;
}
