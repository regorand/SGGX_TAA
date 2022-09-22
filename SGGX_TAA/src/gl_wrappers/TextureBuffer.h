#pragma once
class TextureBuffer
{
private:
	unsigned int m_target;
	unsigned int m_texture_target;
	size_t m_datatype;

	unsigned int m_texBufGlId;
	unsigned int m_bufObjGlId;
public:
	TextureBuffer(void* data, unsigned int size, unsigned int texture_target, size_t data_type, bool dynamic = false);
	~TextureBuffer();

	void bind();
	void unbind();
};

