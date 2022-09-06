#pragma once

#include "../geometry/mesh_object.h"

#include "../geometry/VoxelGrid.h"

#include "../geometry/Octree.h"
#include "RasterizationObject.h"

class SceneObject
{
private:
	Mesh_Object_t mMeshObj;

	std::shared_ptr<RasterizationObject> m_rasterization_object;
	std::shared_ptr<RasterizationObject> m_test_tiny_obj;
	std::shared_ptr<VoxelGrid> m_voxels;
	
	std::shared_ptr<Octree> m_octree;
	std::shared_ptr<RasterizationObject> m_octreeVisObject;

public:
	SceneObject();

	bool hasRasterizationObject();
	bool hasVoxels();
	bool hasRenderableVoxels();
	bool hasOctree();
	bool hasRenderableOctree();
	bool hasRenderableOctreeVisualization();

	void setMeshObject(Mesh_Object_t &mesh);
	Mesh_Object_t& getMeshObject();	

	void registerRasterizationObject(std::shared_ptr<RasterizationObject> rasterization_object);

	void registerVoxels(std::shared_ptr<VoxelGrid> voxels);

	void registerOctree(std::shared_ptr<Octree> octree);

	void initVoxels();
	void unInitVoxels();

	void initOctree();
	void unInitOctree();

	void reloadShaders();

	std::shared_ptr<RasterizationObject> getRasterizationObject();
	std::shared_ptr<VoxelGrid> getVoxels();
	std::shared_ptr<Octree> getOctree();
	std::shared_ptr<RasterizationObject> getOctreeVisObject();
};

