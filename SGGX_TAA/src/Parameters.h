#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
//#include <string>

#include "Types.h"

void initParams();

typedef struct params_s {
	float fov = glm::pi<float>() / 3;
	int windowWidth = 1920;
	int windowHeight = 1080;

	
	int current_shader_output = 1;

	float camera_dist = 4.0;
	float camera_height = 0;
	float camera_rotation[2] = { 0, 0 };

	std::string active_shader_output = "";
	int active_shader_output_index = 0;
	std::string active_render_type = "";
	std::string selected_file = "";

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