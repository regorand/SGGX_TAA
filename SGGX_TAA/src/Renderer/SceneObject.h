#pragma once

#include "../geometry/mesh_object.h"

#include "../geometry/VoxelGrid.h"
#include "RasterizationObject.h"

class SceneObject
{
private:
	std::shared_ptr<RasterizationObject> m_rasterization_object;
	std::shared_ptr<VoxelGrid> m_voxels;

public:
	SceneObject();

	bool hasRasterizationObject();
	bool hasVoxels();

	void registerRasterizationObject(std::shared_ptr<RasterizationObject> &rasterization_object);
	void registerVoxels(std::shared_ptr<VoxelGrid> &voxels);

	void reloadShaders();

	std::shared_ptr<RasterizationObject> getRasterizationObject();
	std::shared_ptr<VoxelGrid> getVoxels();
};

