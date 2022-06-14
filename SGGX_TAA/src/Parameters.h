#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

typedef struct params_s {
	float fov = glm::pi<float>() / 3;
	int windowWidth = 1600;
	int windowHeight = 900;

	float camera_dist = 10.0;
	float rotation[3] = { 0, 0, 0 };
	float light_direction[2] = { 0, 0 };
	bool printUniformNotFound = false;
	bool showAABBOutline = false;
	bool renderRayMarch = true;
	bool rotateCamera = false;
	int num_ggx_samples = 4;
	float ggx_param = 0.2f;
	float rayMarchDist = 0.1f;
	int rayMarchMaxSteps = 80;
} Parameters;

extern Parameters parameters;