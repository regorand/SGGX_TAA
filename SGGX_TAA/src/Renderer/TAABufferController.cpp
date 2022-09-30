#include "TAABufferController.h"

TAABufferController::TAABufferController() {}

TAABufferController::~TAABufferController() {}

void TAABufferController::registerBuffer(std::shared_ptr<TextureBuffer>& target, const size_t size, const unsigned int datatype, const unsigned int texture_target)
{
	uint8_t* buffer = new uint8_t[size];
	memset(buffer, 0, size);
	target = std::make_shared<TextureBuffer>(buffer, size, texture_target, datatype);
	delete buffer;
}

void TAABufferController::registerBuffers(const unsigned int width, const unsigned int height)
{
	const unsigned int render_texture_target = 0;
	const unsigned int history_texture_target = 1;
	const unsigned int rejection_texture_target = 2;
	const unsigned int node_hit_texture_target = 3;
	const unsigned int space_hit_texture_target = 4;
	const unsigned int motion_vector_texture_target = 5;
	const unsigned int lod_diff_texture_target = 6;

	const unsigned int buffer_element_size = width * height;

	size_t render_buffer_size = 4 * sizeof(float) * buffer_element_size;
	registerBuffer(m_renderBuffer, render_buffer_size, GL_RGBA32F, render_texture_target);

	size_t history_buffer_size = 4 * sizeof(float) * buffer_element_size;
	registerBuffer(m_historyBuffer, history_buffer_size, GL_RGBA32F, history_texture_target);

	size_t rejection_buffer_size = 4 * sizeof(uint32_t) * buffer_element_size;
	registerBuffer(m_rejectionBuffer, rejection_buffer_size, GL_RGBA32UI, rejection_texture_target);

	size_t node_hit_buffer_size = 4 * sizeof(uint32_t) * buffer_element_size;
	registerBuffer(m_nodeHitBuffer, node_hit_buffer_size, GL_R32UI, node_hit_texture_target);

	size_t space_hit_buffer_size = 4 * sizeof(float) * buffer_element_size;
	registerBuffer(m_spaceHitBuffer, space_hit_buffer_size, GL_RGBA32F, space_hit_texture_target);

	size_t motion_vector_buffer_size = 4 * sizeof(float) * buffer_element_size;
	registerBuffer(m_motionVectorBuffer, motion_vector_buffer_size, GL_RGBA32F, motion_vector_texture_target);

	size_t lod_diff_buffer_size = 1 * sizeof(float) * buffer_element_size;
	registerBuffer(m_estimatedLodDiff, lod_diff_buffer_size, GL_R32F, lod_diff_texture_target);

	m_isValid = true;
}

bool TAABufferController::isValid()
{
	return m_isValid;
}

void TAABufferController::bindAll()
{
	m_renderBuffer->bind();
	m_historyBuffer->bind();
	m_rejectionBuffer->bind();
	m_nodeHitBuffer->bind();
	m_spaceHitBuffer->bind();	
	m_motionVectorBuffer->bind();
	m_estimatedLodDiff->bind();
}

void TAABufferController::unbindAll()
{
	m_renderBuffer->unbind();
	m_historyBuffer->unbind();
	m_rejectionBuffer->unbind();
	m_nodeHitBuffer->unbind();
	m_spaceHitBuffer->unbind();
	m_motionVectorBuffer->unbind();
	m_estimatedLodDiff->unbind();
}
