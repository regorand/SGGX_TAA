#include "Renderer.h"


Renderer::Renderer()
{
	projectionMatrix = glm::mat4(1);
}

void Renderer::render(RasterizationObject *object, Camera &camera, std::vector<std::shared_ptr<Light>> &lights)
{
	glm::mat4 MVP = projectionMatrix * camera.getViewMatrix() * object->getLocalTransform();

	object->getVertexArray()->bind();
	object->getIndexBuffer()->bind();
	object->getShader()->bind();
	object->getShader()->setUniformMat4f("Model_Matrix", object->getLocalTransform());
	object->getShader()->setUniformMat4f("MVP_matrix", MVP);

	object->getShader()->setUniform1i("output_type", parameters.active_shader_output_index);

	auto materials = object->getMaterials();

	

	//glm::vec3 lightDirection = glm::vec3(glm::cos(parameters.light_direction[0]) * glm::cos(parameters.light_direction[1]), 
	//	glm::sin(parameters.light_direction[1]), 
	//	glm::sin(parameters.light_direction[0]) * glm::cos(parameters.light_direction[1]));

	glm::vec3 lightDirection = glm::vec3(1, 0, 0);
	object->getShader()->setUniform3f("light_direction", lightDirection);
	
	std::vector<glm::vec3> lights_pos;
	std::vector<glm::vec3> lights_intensities;
	for (auto &light : lights) {
		lights_pos.push_back(light->getPosition());
		lights_intensities.push_back(light->getLightColor());
	}
	object->getShader()->setUniform1i("num_lights", lights.size());
	object->getShader()->setUniform3fv("light_positions", lights.size(), lights_pos.data());
	object->getShader()->setUniform3fv("lights_intensities", lights.size(), lights_intensities.data());

	if (object->getTextures().size() != 0) {
		
	}
	std::vector<std::shared_ptr<Texture>> textures = object->getTextures();
	unsigned int num_textures = glm::min(textures.size(), MAX_TEXTURES);
	for (size_t i = 0; i < num_textures; i++) {
		object->getTextures()[i]->Bind(i);
		object->getShader()->setUniform1i("textures[" + std::to_string(i) + "]", i);
	}

	if (materials.size() > 0) {
		object->getShader()->setUniform3f("K_A", materials[0].m_KA);
		object->getShader()->setUniform3f("K_D", materials[0].m_KD);
		object->getShader()->setUniform3f("K_S", materials[0].m_KS);
		object->getShader()->setUniform1f("spec_exponent", materials[0].m_SpecularCoeff);
	}

	object->getShader()->setUniform3f("camera_pos", camera.getPosition());

	object->getShader()->setUniform1f("ggx_parameter", parameters.ggx_param);
	object->getShader()->setUniform1i("samples", parameters.num_ggx_samples);

	GLPrintError();
	glDrawElements(GL_TRIANGLES, object->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr);
	GLPrintError();

	object->getShader()->unbind();
	object->getIndexBuffer()->unbind();
	object->getVertexArray()->unbind();
}

void Renderer::renderVoxels(RayMarchObject* object, Camera& camera, VoxelGrid& voxels)
{
	object->getVertexArray()->bind();
	object->getArrayBuffer()->bind();
	auto shader = object->getVoxelShader();
	voxels.bindBuffers();
	shader->bind();

	shader->setUniform1i("output_type", parameters.active_shader_output_index);

	glm::mat4 transformation_matrix = camera.getViewMatrix();

	shader->setUniform3f("camera_pos", camera.getPosition());

	shader->setUniform1i("dimension", voxels.getDimension());
	shader->setUniform1f("voxel_size", voxels.getVoxelSize());
	shader->setUniform3f("lower", voxels.getLower());
	shader->setUniform3f("higher", voxels.getHigher());
	shader->setUniform1f("AABBOutlineFactor", parameters.renderVoxelsAABB ? 1.0f : 0.0f);
	

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	voxels.unbindBuffers();
	object->getVertexArray()->unbind();
	object->getArrayBuffer()->unbind();
	shader->unbind();
}

void Renderer::renderOctreeVisualization(RasterizationObject* object, Camera& camera)
{
	glm::mat4 MVP = projectionMatrix * camera.getViewMatrix() * object->getLocalTransform();

	// line sizes from 0.5 to 10
	// step size: 0.125
	glLineWidth(1.0f);

	object->getVertexArray()->bind();
	object->getIndexBuffer()->bind();
	object->getShader()->bind();

	object->getShader()->setUniformMat4f("Model_Matrix", object->getLocalTransform());
	object->getShader()->setUniformMat4f("MVP_matrix", MVP);

	glDrawElements(GL_LINES, object->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr);

	object->getShader()->unbind();
	object->getIndexBuffer()->unbind();
	object->getVertexArray()->unbind();
}

void Renderer::renderOctree(RayMarchObject* object, Camera& camera, Octree& octree)
{
	object->getVertexArray()->bind();
	object->getArrayBuffer()->bind();
	auto shader = object->getOctreeShader();
	shader->bind();

	octree.bindBuffers();	

	glm::mat4 transformation_matrix = camera.getViewMatrix();
	shader->setUniform3f("camera_pos", camera.getPosition());
	
	shader->setUniform3f("lower", octree.getTreeLower());
	shader->setUniform3f("higher", octree.getTreeHigher());
	
	shader->setUniform1i("output_type", parameters.active_shader_output_index);
	shader->setUniform1f("AABBOutlineFactor", parameters.renderVoxelsAABB ? 1.0f : 0.0f);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	octree.unbindBuffers();
	object->getVertexArray()->unbind();
	object->getArrayBuffer()->unbind();
	shader->unbind();
}

void Renderer::setProjection(glm::mat4 projectionMatrix)
{
	this->projectionMatrix = projectionMatrix;
}
