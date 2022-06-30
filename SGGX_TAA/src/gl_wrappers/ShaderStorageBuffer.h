#pragma once

#include "../GL_Utils.h"

class ShaderStorageBuffer
{
private:
	unsigned int m_GlId;
public:
	ShaderStorageBuffer(void* data, unsigned int size, unsigned int target, bool dynamic = false);
	~ShaderStorageBuffer();

	unsigned int getGlId();

	void bind();
	void unbind();
};

