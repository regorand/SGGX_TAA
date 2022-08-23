#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
//#include <string>

#include "Types.h"

void initParams();

typedef struct camera_params_s {
	float fov = glm::pi<float>() / 3;

	float lookAtPos[3] = { 0, 0, 0 };
	float cameraPos[2] = { 0, 0 }; // Position of camera around lookAtPos as angles of unit sphere
	float camera_dist = 4.0; // distance from look at Point
	float angle_speed = 0.01f;

	bool rotateAzimuth = false;
	bool rotatePolar = false;
} Camera_Params;

typedef struct octree_params_s {
	int min_render_depth = 0;
	int max_render_depth = 8;

	// At a depth greater than the max buffer size specified in shader (at the time of this writing 32)
	// undefined behaviour might occur in shader
	int max_tree_depth = 6;

	int roentgen_denominator = 50;

	int num_iterations = 32;

	bool auto_lod = false;

	// terrible name but will be deleted anyways
	bool compare_memory = false;

	bool new_building = true;
} Octree_Params;

typedef struct params_s {
	/* Window */
	int windowWidth = 1920;
	int windowHeight = 1080;

	int current_shader_output = 1;

	int voxel_count = 30;
	bool forceReload;
	bool renderVoxelsAABB = false;

	int min_visualization_depth = 0;
	int max_visualization_depth = 6;

	bool flat_shade = true;

	int current_render_type_index = 0;
	int old_render_type_index = current_render_type_index;
	int current_shader_output_index = 0;

	std::string active_shader_output = "";
	std::string selected_file = "";

	float roughness = 0.3;

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

extern Camera_Params camera_params;

extern Octree_Params octree_params;