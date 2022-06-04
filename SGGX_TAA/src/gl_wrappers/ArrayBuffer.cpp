#include "ArrayBuffer.h"
#include <GL/glew.h>


ArrayBuffer::ArrayBuffer(void* data, unsigned int size, bool dynamic): m_GlId(0)
{
    glGenBuffers(1, &m_GlId);
    glBindBuffer(GL_ARRAY_BUFFER, m_GlId);
    if (dynamic) {
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
    }
    else {
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    }
}

ArrayBuffer::~ArrayBuffer()
{
    glDeleteBuffers(1, &m_GlId);
}

void ArrayBuffer::updateData(void* data, unsigned int size)
{
    bind();
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
    unbind();
}

void ArrayBuffer::bind()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_GlId);

}

void ArrayBuffer::unbind()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
