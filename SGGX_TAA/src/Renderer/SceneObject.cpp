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

void SceneObject::setMeshObject(Mesh_Object_t &mesh)
{
	mMeshObj = mesh;
}

Mesh_Object_t& SceneObject::getMeshObject()
{
	return mMeshObj;
}

void SceneObject::registerRasterizationObject(std::shared_ptr<RasterizationObject> &rasterization_object)
{
	m_rasterization_object = rasterization_object;
}

void SceneObject::registerVoxels(std::shared_ptr<VoxelGrid> &voxels)
{
	m_voxels = voxels;
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

void SceneObject::reloadShaders() {
	m_rasterization_object->getShader()->reloadShader();
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
