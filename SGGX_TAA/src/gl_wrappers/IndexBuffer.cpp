#include "IndexBuffer.h"

#include <GL/glew.h>
#include "../utils/GL_Utils.h"

IndexBuffer::IndexBuffer(unsigned int* data, unsigned int elem_count): m_GLId(0), count(0)
{
    glGenBuffers(1, &m_GLId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_GLId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elem_count * sizeof(unsigned int), data, GL_STATIC_DRAW);
    this->count = elem_count;
}

IndexBuffer::~IndexBuffer()
{
    glDeleteBuffers(1, &m_GLId);
}

void IndexBuffer::bind()
{
    GLPrintError();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_GLId);
    GLPrintError();
}

void IndexBuffer::unbind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

unsigned int IndexBuffer::getCount()
{
    return count;
}
