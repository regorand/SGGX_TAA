#include "Renderer.h"
#include "../geometry/Utils.h"

Renderer::Renderer()
{
	projectionMatrix = glm::mat4(1);
	GLint data[4];
	glGetIntegerv(GL_VIEWPORT, data);
	m_horizontal_view_port_size = data[2];
	m_vertical_view_port_size = data[3];

	// this should always be a power of 6, since those retain nice properties 
	// of the halton sequence for loop arounds
	const size_t halton_count = 6;

	m_halton_vectors.resize(halton_count);
	for (size_t i = 0; i < halton_count; i++) {
		// m_halton_vectors[i] = glm::vec2(createHaltonSequence(i + 1, 2), createHaltonSequence(i + 1, 3));
		m_halton_vectors[i] = glm::vec2(createHaltonSequence(i + 1, 2), createHaltonSequence(i + 1, 3)) - glm::vec2(0.5);
	}

	m_bufferController.registerBuffers(m_horizontal_view_port_size, m_vertical_view_port_size);
	/*
	const unsigned int history_buffer_target = 4;
	const unsigned int rejection_buffer_target = 5;

	const unsigned int history_texture_target = 0;
	const unsigned int rejection_texture_target = 1;



	size_t history_buffer_size = 4 * sizeof(float) * m_horizontal_view_port_size * m_vertical_view_port_size;;
	float* history_buffer = new float[history_buffer_size];
	memset(history_buffer, 0, history_buffer_size);
	m_historyBuffer = std::make_shared<TextureBuffer>(history_buffer, history_buffer_size, history_buffer_target, history_texture_target, GL_RGBA32F);
	delete history_buffer;

	size_t rejection_buffer_size = 4 * sizeof(uint32_t) * m_horizontal_view_port_size * m_vertical_view_port_size;
	uint32_t* rejection_buffer = new uint32_t[rejection_buffer_size];
	memset(rejection_buffer, 0, rejection_buffer_size);
	m_rejectionBuffer = std::make_shared<TextureBuffer>(rejection_buffer, rejection_buffer_size, rejection_buffer_target, rejection_texture_target, GL_RGBA32UI);
	delete rejection_buffer;
	*/
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
	m_frameCount++;
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
	m_frameCount++;
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

	// If i need different color for octree
	// glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	object->getVertexArray()->bind();
	object->getIndexBuffer()->bind();
	object->getShader()->bind();

	object->getShader()->setUniformMat4f("Model_Matrix", object->getLocalTransform());
	object->getShader()->setUniformMat4f("MVP_matrix", MVP);

	object->getShader()->setUniform1i("output_type", parameters.current_shader_output_index);


	glDrawElements(GL_LINES, object->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr);
	m_frameCount++;

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

	glm::vec3 pos = camera.getPosition();
	glm::mat view_mat = camera.getViewMatrix();
	glm::vec3 view_dir = camera.getViewDirection();


	m_bufferController.bindAll();
	//m_rejectionBuffer->bind();
	//m_historyBuffer->bind();

	bool bound = octree.bindBuffers();
	if (bound) {
		glm::mat4 transformation_matrix = camera.getViewMatrix();
		shader->setUniform3f("camera_pos", camera.getPosition());
		shader->setUniform3f("camera_view_dir", camera.getViewDirection());

		shader->setUniform3f("lower", octree.getTreeLower());
		shader->setUniform3f("higher", octree.getTreeHigher());

		shader->setUniformMat4f("view_matrix", camera.getViewMatrix());

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

		shader->setUniform1i("render_buffer", 0);
		shader->setUniform1i("history_buffer", 1);
		shader->setUniform1i("rejection_buffer", 2);
		shader->setUniform1i("node_hit_buffer", 3);
		shader->setUniform1i("space_hit_buffer", 4);
		shader->setUniform1i("motion_vector_buffer", 5);
		shader->setUniform1i("lod_diff_buffer", 6);

		shader->setUniform1i("auto_lod", octree_params.auto_lod ? 1 : 0);

		shader->setUniform1i("LoD_feedback_types", taa_params.Lod_feedback_level);
		shader->setUniform1i("max_lod_diff", taa_params.max_lod_diff);
		shader->setUniform1i("apply_lod_offset", taa_params.apply_lod_offset ? 1 : 0);
		shader->setUniform1i("visualize_feedback_level", taa_params.visualize_feedback_level ? 1 : 0);

		shader->setUniform3f("up_vector", camera.getUpVector());

		shader->setUniform1i("num_iterations", octree_params.num_iterations);

		shader->setUniform1i("max_tree_depth", octree.getMaxDepth());

		shader->setUniform1i("min_render_depth", octree_params.min_render_depth);
		// shader->setUniform1i("max_render_depth", octree_params.max_render_depth);

		octree_params.render_depth = glm::min(octree_params.render_depth, (float)octree.getMaxDepth());
		shader->setUniform1f("render_depth", octree_params.render_depth);
		shader->setUniform1i("smooth_lod", octree_params.smooth_lod ? 1 : 0);

		shader->setUniform1i("roentgen_denom", octree_params.roentgen_denominator);

		shader->setUniform1i("output_type", parameters.current_shader_output_index);
		shader->setUniform1f("AABBOutlineFactor", parameters.renderVoxelsAABB ? 1.0f : 0.0f);

		shader->setUniform1i("nodes_size", octree.getNodesSize());
		shader->setUniform1i("inner_nodes_size", octree.getInnerSize());
		shader->setUniform1i("leaves_size", octree.getLeavesSize());

		if (taa_params.taa_active && !taa_params.disable_jiggle) {
			glm::vec2 jiggle = m_halton_vectors[m_frameCount % 6] * glm::vec2(1.0f / horizontal_pixels, 1.0f / vertical_pixels);
			shader->setUniform2f("jiggle_offset", jiggle * taa_params.jiggle_factor);
		}
		else {
			shader->setUniform2f("jiggle_offset", glm::vec2(0));
		}

		shader->setUniform1f("diffuse_parameter", parameters.diffuse_parameter);
		shader->setUniform1i("interpolate_voxels", taa_params.interpolate_voxels ? 1 : 0);

		shader->setUniform1i("history_parent_level", taa_params.historyParentRejectionLevel);

		float s1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float s2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		shader->setUniform1f("seed1", s1);
		shader->setUniform1f("seed2", s2);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		if (taa_params.taa_active && object->hasValidTAAShader()) {


			glDisable(GL_DEPTH_TEST);


			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			// glFinish();

			shader->unbind();

			auto taa_resolve_shader = object->getTAAResolveShader();

			taa_resolve_shader->bind();

			taa_resolve_shader->setUniform1i("horizontal_pixels", horizontal_pixels);
			taa_resolve_shader->setUniform1i("vertical_pixels", vertical_pixels);

			taa_resolve_shader->setUniform1i("history_rejection_active", taa_params.doHistoryRejection ? 1 : 0);
			taa_resolve_shader->setUniform1i("visualize_history_rejection", taa_params.visualizeHistoryRejection ? 1 : 0);
			taa_resolve_shader->setUniform1f("taa_alpha", taa_params.alpha);
			taa_resolve_shader->setUniform1i("history_buffer_depth", taa_params.historyRejectionBufferDepth);
			taa_resolve_shader->setUniform1i("interpolate_alpha", taa_params.interpolate_alpha);
			taa_resolve_shader->setUniform1i("history_parent_level", taa_params.historyParentRejectionLevel);

			taa_resolve_shader->setUniform1i("do_reprojection", taa_params.do_reprojection ? 1 : 0);

			taa_resolve_shader->setUniform1i("visualize_active_alpha", taa_params.visualize_active_alpha ? 1 : 0);
			taa_resolve_shader->setUniform1i("visualize_edge_detection", taa_params.visualize_edge_detection ? 1 : 0);
			taa_resolve_shader->setUniform1i("visualize_motion_vectors", taa_params.visualize_motion_vectors ? 1 : 0);

			// TODO: find out which buffers are actually needed for which passes
			taa_resolve_shader->setUniform1i("render_buffer", 0);
			taa_resolve_shader->setUniform1i("history_buffer", 1);
			taa_resolve_shader->setUniform1i("rejection_buffer", 2);
			taa_resolve_shader->setUniform1i("node_hit_buffer", 3);
			taa_resolve_shader->setUniform1i("space_hit_buffer", 4);
			taa_resolve_shader->setUniform1i("motion_vector_buffer", 5);
			taa_resolve_shader->setUniform1i("lod_diff_buffer", 6);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			taa_resolve_shader->unbind();
		}


		m_frameCount++;
	}

	octree.unbindBuffers();

	//m_historyBuffer->unbind();
	//m_rejectionBuffer->unbind();
	m_bufferController.unbindAll();

	object->getVertexArray()->unbind();
	object->getArrayBuffer()->unbind();
	shader->unbind();
}

glm::vec2 Renderer::getViewPortDimensions()
{
	return glm::vec2(m_horizontal_view_port_size, m_vertical_view_port_size);
}

void Renderer::setProjection(glm::mat4 projectionMatrix)
{
	this->projectionMatrix = projectionMatrix;
}
