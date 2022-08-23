#pragma once
class TextureBuffer
{
private:
	unsigned int m_target;

	unsigned int m_texBufGlId;
	unsigned int m_bufObjGlId;
public:
	TextureBuffer(void* data, unsigned int size, unsigned int target, bool dynamic = false);
	~TextureBuffer();

	unsigned int getGlId();

	void bind();
	void unbind();
};

