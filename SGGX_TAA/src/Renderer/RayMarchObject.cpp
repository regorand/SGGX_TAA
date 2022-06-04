#include "RayMarchObject.h"

RayMarchObject::RayMarchObject(std::shared_ptr<VertexArray> vao, std::shared_ptr<ArrayBuffer> abo, std::shared_ptr<Shader> shader)
	: vao(vao), abo(abo), shader(shader)
{}

RayMarchObject::~RayMarchObject()
{}

void RayMarchObject::reloadShader()
{
	shader->reloadShader();
}

std::shared_ptr<VertexArray> RayMarchObject::getVertexArray()
{
	return vao;
}

std::shared_ptr<ArrayBuffer> RayMarchObject::getArrayBuffer()
{
	return abo;
}

std::shared_ptr<Shader> RayMarchObject::getShader()
{
	return shader;
}
