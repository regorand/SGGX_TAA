#pragma once

#include <memory>

#include "../gl_wrappers/VertexArray.h"
#include "../gl_wrappers/ArrayBuffer.h"
#include "../gl_wrappers/Shader.h"


class RayMarchObject
{
private:
	std::shared_ptr<VertexArray> vao;
	std::shared_ptr<ArrayBuffer> abo;
	std::shared_ptr<Shader> shader;

public: 
	RayMarchObject(std::shared_ptr<VertexArray> vao, std::shared_ptr<ArrayBuffer> abo, std::shared_ptr<Shader> shader);
	~RayMarchObject();

	void reloadShader();

	std::shared_ptr<VertexArray> getVertexArray();
	std::shared_ptr<ArrayBuffer> getArrayBuffer();
	std::shared_ptr<Shader> getShader();
};

