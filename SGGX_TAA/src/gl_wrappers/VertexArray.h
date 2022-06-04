#pragma once

#include "ArrayBuffer.h"
#include "../GL_Utils.h"

class VertexArray
{
private:
	unsigned int m_GlId;

public:
	VertexArray();
	~VertexArray();

	void bind();
	void unbind();
};

