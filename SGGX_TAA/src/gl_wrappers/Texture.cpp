#include "Texture.h"
#include "../utils/GL_Utils.h"

#include "../3rd_party/std_image/std_image.h"	

Texture::Texture(const std::string& path)
	:m_GlId(), m_FilePath(path), m_bytesPerPixel(0), m_height(0), m_width(0), m_Buf(nullptr)
{
	stbi_set_flip_vertically_on_load(1);
	m_Buf = stbi_load(path.c_str(), &m_width, &m_height, &m_bytesPerPixel, 4);

	glGenTextures(1, &m_GlId);
	glBindTexture(GL_TEXTURE_2D, m_GlId);

	// Todo (How to) Actually Filter here ?
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_Buf);
	Unbind();

	if (m_Buf) {
		stbi_image_free(m_Buf);
	}
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_GlId);
}

void Texture::Bind(unsigned int textureSlot)
{
	glActiveTexture(GL_TEXTURE0 + textureSlot);
	glBindTexture(GL_TEXTURE_2D, m_GlId);
}
void Texture::Unbind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}
