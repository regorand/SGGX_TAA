#include "RayMarchObject.h"

RayMarchObject::RayMarchObject(std::shared_ptr<VertexArray> vao, std::shared_ptr<ArrayBuffer> abo)
	: vao(vao), abo(abo), voxel_shader(nullptr), octree_shader(nullptr)
{}

RayMarchObject::~RayMarchObject()
{}

void RayMarchObject::reloadShader()
{
	if (hasVoxelShader()) voxel_shader->reloadShader();
	if (hasOctreeShader()) octree_shader->reloadShader();
}

std::shared_ptr<VertexArray> RayMarchObject::getVertexArray()
{
	return vao;
}

std::shared_ptr<ArrayBuffer> RayMarchObject::getArrayBuffer()
{
	return abo;
}

std::shared_ptr<Shader> RayMarchObject::getVoxelShader()
{
	return voxel_shader;
}

bool RayMarchObject::registerVoxelShader(std::shared_ptr<Shader> shader)
{
	if (shader->isValid()) {
		voxel_shader = shader;
		return true;
	}
	return false;
}

bool RayMarchObject::hasVoxelShader()
{
	return voxel_shader != nullptr && voxel_shader->isValid();
}

std::shared_ptr<Shader> RayMarchObject::getOctreeShader()
{
	return octree_shader;
}

bool RayMarchObject::registerOctreeShader(std::shared_ptr<Shader> shader)
{
	octree_shader = shader;
	return true;
}

bool RayMarchObject::hasOctreeShader()
{
	return octree_shader != nullptr;
}

bool RayMarchObject::hasValidOctreeShader()
{
	return octree_shader != nullptr && octree_shader->isValid();
}
