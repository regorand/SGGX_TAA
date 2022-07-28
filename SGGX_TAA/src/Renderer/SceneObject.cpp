#include "SceneObject.h"

SceneObject::SceneObject()
	: m_rasterization_object(nullptr), m_voxels(nullptr)
{}

bool SceneObject::hasRasterizationObject()
{
	return m_rasterization_object != nullptr;
}

bool SceneObject::hasVoxels()
{
	return m_voxels != nullptr;
}

bool SceneObject::hasRenderableVoxels()
{
	return m_voxels != nullptr && m_voxels->isInit();
}

bool SceneObject::hasOctree()
{
	return m_octree != nullptr;
}

bool SceneObject::hasRenderableOctree()
{
	return m_octree != nullptr && m_octree->isInitiliazed() && m_octree->buffersInitialized();
}

bool SceneObject::hasRenderableOctreeVisualization()
{
	return m_octreeVisObject != nullptr && m_octreeVisObject->isRenderable();
}

void SceneObject::setMeshObject(Mesh_Object_t &mesh)
{
	mMeshObj = mesh;
}

Mesh_Object_t& SceneObject::getMeshObject()
{
	return mMeshObj;
}

void SceneObject::registerRasterizationObject(std::shared_ptr<RasterizationObject> rasterization_object)
{
	m_rasterization_object = rasterization_object;
}

void SceneObject::registerVoxels(std::shared_ptr<VoxelGrid> voxels)
{
	m_voxels = voxels;
}

void SceneObject::registerOctree(std::shared_ptr<Octree> octree)
{
	m_octree = octree;
}

void SceneObject::initVoxels()
{
	if (m_voxels && !m_voxels->isInit()) {
		m_voxels->initBuffers();
	}
}

void SceneObject::unInitVoxels()
{
	if (m_voxels && m_voxels->isInit()) {
		m_voxels->deleteBuffers();
	}
}

void SceneObject::initOctree()
{
	if (m_octree != nullptr) {
		m_octree->initBuffers();
		m_octree->unbindBuffers();

		m_octreeVisObject = nullptr;
		m_octreeVisObject = m_octree->initVisBufferObject(); 
	}
}

void SceneObject::unInitOctree()
{
}

void SceneObject::reloadShaders() {
	m_rasterization_object->getShader()->reloadShader();
	m_octreeVisObject->getShader()->reloadShader();
	// TODO currently don't reload voxel shader here because that is stored at different location
}

std::shared_ptr<RasterizationObject> SceneObject::getRasterizationObject()
{
	return m_rasterization_object;
}

std::shared_ptr<VoxelGrid> SceneObject::getVoxels()
{
	return m_voxels;
}

std::shared_ptr<Octree> SceneObject::getOctree()
{
	return m_octree;
}

std::shared_ptr<RasterizationObject> SceneObject::getOctreeVisObject()
{
	return m_octreeVisObject;
}
