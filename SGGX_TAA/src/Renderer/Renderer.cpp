#include "Renderer.h"


Renderer::Renderer()
{
	projectionMatrix = glm::mat4(1);
	GLint data[4];
	glGetIntegerv(GL_VIEWPORT, data);
	m_horizontal_view_port_size = data[2];
	m_vertical_view_port_size = data[3];

	size_t buf_size = 4 * sizeof(float) * m_horizontal_view_port_size * m_vertical_view_port_size;
	float* tex_coords_buffer = new float[buf_size];

	memset(tex_coords_buffer, 0x3f800000, buf_size);

	tex_coords_buffer[0] = 1;
	tex_coords_buffer[1] = 0;
	tex_coords_buffer[2] = 1;
	tex_coords_buffer[3] = 1;

	const unsigned int tex_buf_target = 4;

	m_bufferTexture = std::make_shared<TextureBuffer>(tex_coords_buffer, buf_size, tex_buf_target);

	delete tex_coords_buffer;
}

void Renderer::render(RasterizationObject* object, Camera& camera, std::vector<std::shared_ptr<Light>>& lights)
{
	glm::mat4 MVP = projectionMatrix * camera.getViewMatrix() * object->getLocalTransform();

	object->getVertexArray()->bind();
	object->getIndexBuffer()->bind();
	object->getShader()->bind();
	object->getShader()->setUniformMat4f("Model_Matrix", object->getLocalTransform());
	object->getShader()->setUniformMat4f("MVP_matrix", MVP);

	object->getShader()->setUniform1i("output_type", parameters.current_shader_output_index);

	auto materials = object->getMaterials();



	//glm::vec3 lightDirection = glm::vec3(glm::cos(parameters.light_direction[0]) * glm::cos(parameters.light_direction[1]), 
	//	glm::sin(parameters.light_direction[1]), 
	//	glm::sin(parameters.light_direction[0]) * glm::cos(parameters.light_direction[1]));

	glm::vec3 lightDirection = glm::vec3(1, 0, 0);
	object->getShader()->setUniform3f("light_direction", lightDirection);

	std::vector<glm::vec3> lights_pos;
	std::vector<glm::vec3> lights_intensities;
	for (auto& light : lights) {
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

	shader->setUniform1i("output_type", parameters.current_shader_output_index);

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

	object->getShader()->setUniform1i("output_type", parameters.current_shader_output_index);


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

	m_bufferTexture->bind();

	bool bound = octree.bindBuffers();
	if (bound) {
		glm::mat4 transformation_matrix = camera.getViewMatrix();
		shader->setUniform3f("camera_pos", camera.getPosition());

		shader->setUniform3f("lower", octree.getTreeLower());
		shader->setUniform3f("higher", octree.getTreeHigher());


		GLint data[4];
		glGetIntegerv(GL_VIEWPORT, data);
		int horizontal_pixels = data[2];
		int vertical_pixels = data[3];

		float horizontal_size;
		float vertical_size;
		object->getSizes(&horizontal_size, &vertical_size);
		float horizontal_pixel_size = horizontal_size / horizontal_pixels;
		float vertical_pixel_size = vertical_size / vertical_pixels;

		shader->setUniform1f("horizontal_pixel_size", horizontal_pixel_size);
		shader->setUniform1f("vertical_pixel_size", vertical_pixel_size);

		shader->setUniform1i("horizontal_pixels", horizontal_pixels);
		shader->setUniform1i("vertical_pixels", vertical_pixels);

		shader->setUniform1i("history_buffer", 0);

		shader->setUniform1i("auto_lod", octree_params.auto_lod ? 1 : 0);

		shader->setUniform1i("num_iterations", octree_params.num_iterations);

		shader->setUniform1i("max_tree_depth", octree.getMaxDepth());

		shader->setUniform1i("min_render_depth", octree_params.min_render_depth);
		shader->setUniform1i("max_render_depth", octree_params.max_render_depth);

		shader->setUniform1i("roentgen_denom", octree_params.roentgen_denominator);

		shader->setUniform1i("output_type", parameters.current_shader_output_index);
		shader->setUniform1f("AABBOutlineFactor", parameters.renderVoxelsAABB ? 1.0f : 0.0f);

		shader->setUniform1i("nodes_size", octree.getNodesSize());
		shader->setUniform1i("inner_nodes_size", octree.getInnerSize());
		shader->setUniform1i("leaves_size", octree.getLeavesSize());

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	}

	octree.unbindBuffers();

	m_bufferTexture->unbind();

	object->getVertexArray()->unbind();
	object->getArrayBuffer()->unbind();
	shader->unbind();
}

void Renderer::setProjection(glm::mat4 projectionMatrix)
{
	this->projectionMatrix = projectionMatrix;
}
