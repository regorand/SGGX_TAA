#pragma once

#include "../geometry/mesh_object.h"

#include "../geometry/VoxelGrid.h"
#include "RasterizationObject.h"

class SceneObject
{
private:
	Mesh_Object_t mMeshObj;

	std::shared_ptr<RasterizationObject> m_rasterization_object;
	std::shared_ptr<VoxelGrid> m_voxels;

public:
	SceneObject();

	bool hasRasterizationObject();
	bool hasVoxels();
	bool hasRenderableVoxels();


	void setMeshObject(Mesh_Object_t &mesh);
	Mesh_Object_t& getMeshObject();

	void registerRasterizationObject(std::shared_ptr<RasterizationObject> &rasterization_object);

	void registerVoxels(std::shared_ptr<VoxelGrid> &voxels);

	void initVoxels();
	void unInitVoxels();

	void reloadShaders();

	std::shared_ptr<RasterizationObject> getRasterizationObject();
	std::shared_ptr<VoxelGrid> getVoxels();
};

