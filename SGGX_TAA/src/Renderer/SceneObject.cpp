#include "SceneObject.h"

SceneObject::SceneObject(std::shared_ptr<VertexArray> vertexArray, 
	std::shared_ptr<IndexBuffer> index_buffer, 
	std::shared_ptr<Shader> shader, 
	glm::mat4 model_matrix, 
	std::vector<Material> materials):
	m_VertexArray(vertexArray), m_Index_buffer(index_buffer), m_Shader(shader), m_ModelMatrix(model_matrix), m_Materials(materials)
{ 
	m_LocalTransformationMatrix = glm::mat4(1);
}

void SceneObject::setLocalTransformation(glm::mat4 local_transformation)
{
	m_LocalTransformationMatrix = local_transformation;
}

std::shared_ptr<VertexArray> SceneObject::getVertexArray() {
	return m_VertexArray;
}

std::shared_ptr<IndexBuffer> SceneObject::getIndexBuffer()
{
	return m_Index_buffer;
}

std::shared_ptr<Shader> SceneObject::getShader()
{
	return m_Shader;
}

std::vector<Material> SceneObject::getMaterials()
{
	return m_Materials;
}

void SceneObject::addArrayBuffer(std::shared_ptr<ArrayBuffer> aBuf)
{
	m_ArrayBuffers.push_back(aBuf);
}

void SceneObject::addTexture(std::shared_ptr<Texture> texture)
{
	m_Textures.push_back(texture);
}


std::vector<std::shared_ptr<Texture>> SceneObject::getTextures()
{
	return m_Textures;
}

void SceneObject::reloadShaders()
{
	m_Shader->reloadShader();
}

glm::mat4 SceneObject::getModelMatrix()
{
	return m_ModelMatrix;
}

glm::mat4 SceneObject::getLocalTransform()
{
	return m_ModelMatrix * m_LocalTransformationMatrix;
}
