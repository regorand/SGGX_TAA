#include "TextureBuffer.h"
#include "../utils/GL_Utils.h"


TextureBuffer::TextureBuffer(void* data, unsigned int size, unsigned int target, unsigned int texture_target, size_t data_type, bool dynamic)
	: m_target(target), m_texture_target(texture_target), m_datatype(data_type)
{
	glGenBuffers(1, &m_bufObjGlId);
	glBindBuffer(GL_TEXTURE_BUFFER, m_bufObjGlId);
	if (dynamic) {
		glBufferData(GL_TEXTURE_BUFFER, size, data, GL_DYNAMIC_DRAW);
	}
	else {
		glBufferData(GL_TEXTURE_BUFFER, size, data, GL_STATIC_DRAW);
	}

	glGenTextures(1, &m_texBufGlId);
	glBindTexture(GL_TEXTURE_BUFFER, m_texBufGlId);

	glTexBuffer(GL_TEXTURE_BUFFER, m_datatype, m_bufObjGlId);
}

TextureBuffer::~TextureBuffer()
{
}


void TextureBuffer::bind()
{
	glBindBuffer(GL_TEXTURE_BUFFER, m_bufObjGlId);

	glBindImageTexture(m_texture_target, m_texBufGlId, 0, GL_FALSE, 0, GL_READ_WRITE, m_datatype);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);
}

void TextureBuffer::unbind()
{
	glBindBuffer(GL_TEXTURE_BUFFER, 0);
}
