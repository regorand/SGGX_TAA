#include "TextureBuffer.h"
#include "../utils/GL_Utils.h"


TextureBuffer::TextureBuffer(void* data, unsigned int size, unsigned int target, bool dynamic)
	: m_target(target)
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

	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, m_bufObjGlId);
}

TextureBuffer::~TextureBuffer()
{
}

unsigned int TextureBuffer::getGlId()
{
	return 0;
}

void TextureBuffer::bind()
{
	glBindBuffer(GL_TEXTURE_BUFFER, m_bufObjGlId);
	glBindTexture(GL_TEXTURE_BUFFER, m_texBufGlId);

	glBindImageTexture(0, m_texBufGlId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
}

void TextureBuffer::unbind()
{
	glBindBuffer(GL_TEXTURE_BUFFER, 0);
	glBindTexture(GL_TEXTURE_BUFFER, 0);
}
