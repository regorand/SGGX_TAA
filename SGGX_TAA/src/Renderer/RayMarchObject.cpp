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
	if (hasTAAShader()) taa_resolve_shader->reloadShader();
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

std::shared_ptr<Shader> RayMarchObject::getTAAResolveShader()
{
	return taa_resolve_shader;
}

bool RayMarchObject::registerTAAShader()
{
	const std::string vert_path = "res/shaders/ray_marching.vert";
	const std::string frag_path = "res/shaders/taa_resolve.frag";

	taa_resolve_shader = std::make_shared<Shader>(vert_path, frag_path);

	return taa_resolve_shader->isValid();
}

bool RayMarchObject::hasTAAShader()
{
	return taa_resolve_shader != nullptr;
}

bool RayMarchObject::hasValidTAAShader()
{
	return taa_resolve_shader != nullptr && taa_resolve_shader->isValid();
}

void RayMarchObject::setSizes(float horizontal_size, float vertical_size)
{
	m_horizontal_size = horizontal_size;
	m_vertical_size = vertical_size;
}

void RayMarchObject::getSizes(float* horizontal_size, float* vertical_size)
{
	*horizontal_size = m_horizontal_size;
	*vertical_size = m_vertical_size;
}
