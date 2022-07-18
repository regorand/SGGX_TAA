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

	if (object->getTextures().size() != 0) {
		object->getTextures()[0]->Bind(0);
		object->getShader()->setUniform1i("diff_texture", 0);
	}

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

	if (materials.size() > 0) {
		object->getShader()->setUniform3f("K_A", materials[0].m_KA);
		object->getShader()->setUniform3f("K_D", materials[0].m_KD);
		object->getShader()->setUniform3f("K_S", materials[0].m_KS);
		object->getShader()->setUniform1f("spec_exponent", materials[0].m_SpecularCoeff);
	}

	object->getShader()->setUniform3f("camera_pos", camera.getPosition());

	object->getShader()->setUniform1f("ggx_parameter", parameters.ggx_param);
	object->getShader()->setUniform1i("samples", parameters.num_ggx_samples);

	glDrawElements(GL_TRIANGLES, object->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr);

	object->getShader()->unbind();
	object->getIndexBuffer()->unbind();
	object->getVertexArray()->unbind();
}

void Renderer::renderRayMarching(RayMarchObject* object, Camera& camera, std::vector<std::shared_ptr<Light>>& lights)
{
	object->getVertexArray()->bind();
	object->getArrayBuffer()->bind();
	object->getShader()->bind();

	glm::mat4 transformation_matrix = camera.getViewMatrix();

	object->getShader()->setUniformMat4f("projection_matrix", projectionMatrix);
	object->getShader()->setUniformMat4f("transformation_matrix", projectionMatrix);

	object->getShader()->setUniform3f("camera_pos", camera.getPosition());

	object->getShader()->setUniform1f("step_distance", parameters.rayMarchDist);
	object->getShader()->setUniform1f("AABBOutlineFactor", parameters.showAABBOutline);
	object->getShader()->setUniform1i("max_steps", parameters.rayMarchMaxSteps);
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	object->getVertexArray()->unbind();
	object->getArrayBuffer()->unbind();
	object->getShader()->unbind();
}

void Renderer::renderVoxels(RayMarchObject* object, Camera& camera, VoxelGrid& voxels)
{
	object->getVertexArray()->bind();
	object->getArrayBuffer()->bind();
	object->getShader()->bind();

	object->getShader()->setUniform1i("output_type", parameters.active_shader_output_index);
	/*
	float len = glm::length(voxels.getLower());
	len = glm::max(len, glm::length(voxels.getHigher()));
	parameters.camera_dist = len * 10;
	camera.update();
	*/

	camera.setPosition(camera.getPosition() + glm::vec3(0, parameters.camera_height, 0));
	glm::mat4 transformation_matrix = camera.getViewMatrix();

	/*
	object->getShader()->setUniformMat4f("projection_matrix", projectionMatrix);
	object->getShader()->setUniformMat4f("transformation_matrix", projectionMatrix);
	*/
	object->getShader()->setUniform3f("camera_pos", camera.getPosition());

	
	object->getShader()->setUniform1i("dimension", voxels.getDimension());
	object->getShader()->setUniform1f("voxel_size", voxels.getVoxelSize());
	object->getShader()->setUniform3f("lower", voxels.getLower());
	object->getShader()->setUniform3f("higher", voxels.getHigher());

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	object->getVertexArray()->unbind();
	object->getArrayBuffer()->unbind();
	object->getShader()->unbind();
}

void Renderer::setProjection(glm::mat4 projectionMatrix)
{
	this->projectionMatrix = projectionMatrix;
}
