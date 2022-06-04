#pragma once
class ArrayBuffer
{
private:
	unsigned int m_GlId;
public:
	ArrayBuffer(void* data, unsigned int size, bool dynamic = false);
	~ArrayBuffer();

	void updateData(void* data, unsigned int size);

	void bind();
	void unbind();
};

