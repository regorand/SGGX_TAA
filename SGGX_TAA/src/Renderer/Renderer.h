#pragma once

// #include <GL/glew.h>
//#include "GL_Utils.h"

#include <glm/ext/matrix_double4x4.hpp>

#include "../gl_wrappers/VertexArray.h"
#include "../gl_wrappers/IndexBuffer.h"
#include "../gl_wrappers/Shader.h"
#include "SceneObject.h"
#include "Camera.h"
#include "Light.h"
#include "RayMarchObject.h"
#include "../geometry/VoxelGrid.h"

class Renderer
{
	glm::mat4 projectionMatrix;
public:
	Renderer();
	~Renderer() {};

	void render(SceneObject* object, Camera &camera, std::vector<std::shared_ptr<Light>> &lights);
	void renderRayMarching(RayMarchObject* object, Camera& camera, std::vector<std::shared_ptr<Light>>& lights);
	void renderVoxels(RayMarchObject* object, Camera& camera, VoxelGrid& voxels);

	void setProjection(glm::mat4 projectionMatrix);
	void setViewMatrix(glm::mat4 viewMatrix);
};

