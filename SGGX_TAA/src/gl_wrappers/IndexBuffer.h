#pragma once

class IndexBuffer
{
private:
	unsigned int m_GLId;
	unsigned int count;

public:
	IndexBuffer(unsigned int *data, unsigned int count);
	~IndexBuffer();

	void bind();
	void unbind();

	unsigned int getCount();
};

