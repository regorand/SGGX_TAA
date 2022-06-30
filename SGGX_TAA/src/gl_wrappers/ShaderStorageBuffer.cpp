#include "ShaderStorageBuffer.h"

ShaderStorageBuffer::ShaderStorageBuffer(void* data, unsigned int size, unsigned int target, bool dynamic)
{
	glGenBuffers(1, &m_GlId);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_GlId);
	if (dynamic) {
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW);
	}
	else {
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_STATIC_DRAW);
	}
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, target, m_GlId);
}

ShaderStorageBuffer::~ShaderStorageBuffer()
{
	glDeleteBuffers(1, &m_GlId);
}

unsigned int ShaderStorageBuffer::getGlId()
{
	return m_GlId;
}

void ShaderStorageBuffer::bind()
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_GlId);
}

void ShaderStorageBuffer::unbind()
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
