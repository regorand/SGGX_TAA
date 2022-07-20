#pragma once

#include <iostream>
#include <inttypes.h>
#include <glm/glm.hpp>
#include <memory>

#include "../gl_wrappers/ShaderStorageBuffer.h"

typedef struct {
	float val;
	float normal_x;
	float normal_y;
	float normal_z;
	
} voxel_t;

class VoxelGrid
{
	glm::vec3 m_lower;
	glm::vec3 m_higher;
	uint16_t m_dimension;

	bool m_isInit = false;

	std::shared_ptr<ShaderStorageBuffer> m_ssb;

	voxel_t* m_data;
private:

public:
	VoxelGrid(uint16_t dimension);
	~VoxelGrid();

	uint16_t getDimension();

	void setLower(glm::vec3 lower);
	void setHigher(glm::vec3 higher);
	glm::vec3 getLower();
	glm::vec3 getHigher();

	float getVoxelSize();
	glm::vec3 getCenterOfVoxel(glm::u16vec3 indices);

	void setVoxel(glm::u16vec3 indices, voxel_t new_voxel);
	voxel_t getVoxel(glm::u16vec3 indices);

	voxel_t* getVoxelData();

	bool isInit();

	void initBuffers();
	void deleteBuffers();

	unsigned int countVoxels();
	unsigned int countNonEmptyVoxels();
private:
	bool checkDimension(glm::u16vec3 indices);
};

