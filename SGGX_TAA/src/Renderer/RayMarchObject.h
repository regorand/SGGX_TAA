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
	//std::shared_ptr<Shader> shader;

	std::shared_ptr<Shader> voxel_shader;
	std::shared_ptr<Shader> octree_shader;
	std::shared_ptr<Shader> taa_resolve_shader;

	float m_horizontal_size;
	float m_vertical_size;

public: 
	RayMarchObject(std::shared_ptr<VertexArray> vao, std::shared_ptr<ArrayBuffer> abo);
	~RayMarchObject();

	void reloadShader();

	std::shared_ptr<VertexArray> getVertexArray();
	std::shared_ptr<ArrayBuffer> getArrayBuffer();
	//std::shared_ptr<Shader> getShader();

	std::shared_ptr<Shader> getVoxelShader();
	bool registerVoxelShader(std::shared_ptr<Shader> shader);
	bool hasVoxelShader();

	std::shared_ptr<Shader> getOctreeShader();
	bool registerOctreeShader(std::shared_ptr<Shader> shader);
	bool hasOctreeShader();
	bool hasValidOctreeShader();

	std::shared_ptr<Shader> getTAAResolveShader();
	bool registerTAAShader();
	bool hasTAAShader();
	bool hasValidTAAShader();

	void setSizes(float horizontal_size, float vertical_size);
	void getSizes(float *horizontal_size, float *vertical_size);
};

