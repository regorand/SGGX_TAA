#include "VertexArray.h"


VertexArray::VertexArray()
{
	unbind();
	glGenVertexArrays(1, &m_GlId);
	glBindVertexArray(m_GlId);
}

VertexArray::~VertexArray()
{
	glDeleteVertexArrays(1, &m_GlId);
}

void VertexArray::bind()
{
	glBindVertexArray(m_GlId);
}

void VertexArray::unbind()
{
	glBindVertexArray(0);
}
