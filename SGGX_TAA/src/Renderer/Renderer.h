#pragma once

// #include <GL/glew.h>
//#include "GL_Utils.h"

#include <glm/ext/matrix_double4x4.hpp>

#include "../gl_wrappers/VertexArray.h"
#include "../gl_wrappers/IndexBuffer.h"
#include "../gl_wrappers/Shader.h"
#include "RasterizationObject.h"
#include "Camera.h"
#include "Light.h"
#include "RayMarchObject.h"
#include "../geometry/VoxelGrid.h"
#include "../geometry/Octree.h"
#include "../gl_wrappers/TextureBuffer.h"

// has corresponding const in phong shader
static const size_t MAX_TEXTURES = 4;

class Renderer
{
	glm::mat4 projectionMatrix;

	unsigned int m_vertical_view_port_size;
	unsigned int m_horizontal_view_port_size;

	size_t m_frameCount = 0;

	std::vector<glm::vec2> m_halton_vectors;

	std::shared_ptr<TextureBuffer> m_historyBuffer;

	// TODO better name ? -> This stores octree node ids, if they dont fit with current -> history rejection
	std::shared_ptr<TextureBuffer> m_rejectionBuffer;
public:
	Renderer();
	~Renderer() {};

	void render(RasterizationObject* object, Camera &camera, std::vector<std::shared_ptr<Light>> &lights);
	void renderVoxels(RayMarchObject* object, Camera& camera, VoxelGrid& voxels);
	void renderOctreeVisualization(RasterizationObject* object, Camera& camera);
	void renderOctree(RayMarchObject* object, Camera& camera, Octree& octree);

	void setProjection(glm::mat4 projectionMatrix);
	void setViewMatrix(glm::mat4 viewMatrix);
};

