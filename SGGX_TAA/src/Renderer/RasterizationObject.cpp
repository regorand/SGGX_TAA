#include "RasterizationObject.h"

RasterizationObject::RasterizationObject(std::shared_ptr<VertexArray> vertexArray,
	std::shared_ptr<IndexBuffer> index_buffer, 
	std::shared_ptr<Shader> shader, 
	glm::mat4 model_matrix, 
	std::vector<Material> materials):
	m_VertexArray(vertexArray), m_Index_buffer(index_buffer), m_Shader(shader), m_ModelMatrix(model_matrix), m_Materials(materials)
{ 
	m_LocalTransformationMatrix = glm::mat4(1);
}

RasterizationObject::~RasterizationObject()
{
	//TODO implement this ?
}

void RasterizationObject::setLocalTransformation(glm::mat4 local_transformation)
{
	m_LocalTransformationMatrix = local_transformation;
}

std::shared_ptr<VertexArray> RasterizationObject::getVertexArray() {
	return m_VertexArray;
}

std::shared_ptr<IndexBuffer> RasterizationObject::getIndexBuffer()
{
	return m_Index_buffer;
}

std::shared_ptr<Shader> RasterizationObject::getShader()
{
	return m_Shader;
}

std::vector<Material> RasterizationObject::getMaterials()
{
	return m_Materials;
}

void RasterizationObject::addArrayBuffer(std::shared_ptr<ArrayBuffer> aBuf)
{
	m_ArrayBuffers.push_back(aBuf);
}

void RasterizationObject::addTexture(std::shared_ptr<Texture> texture)
{
	m_Textures.push_back(texture);
}


std::vector<std::shared_ptr<Texture>> RasterizationObject::getTextures()
{
	return m_Textures;
}

void RasterizationObject::reloadShaders()
{
	m_Shader->reloadShader();
}

glm::mat4 RasterizationObject::getModelMatrix()
{
	return m_ModelMatrix;
}

glm::mat4 RasterizationObject::getLocalTransform()
{
	return m_ModelMatrix * m_LocalTransformationMatrix;
}
