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

	bool doCameraPath = false;
	bool autoplay = false;
	int min_frame = 0;
	int max_frame = 1;
	int current_frame = 0;

	int add_frame = 0;
	bool lock_to_path = false;
} Camera_Params;

typedef struct octree_params_s {
	int min_render_depth = 0;
	//int max_render_depth = 8;
	float render_depth = 8;
	bool smooth_lod = false;

	// At a depth greater than the max buffer size specified in shader (at the time of this writing 32)
	// undefined behaviour might occur in shader
	int max_tree_depth = 6;

	int roentgen_denominator = 50;

	int num_iterations = 32;

	float roughness = 0.3;

	bool auto_lod = false;

	bool new_building = true;
} Octree_Params;

typedef struct taa_params_s {
	bool taa_active = false;
	bool disable_jiggle = false;
	
	float alpha = 0.1;
	
	bool doHistoryRejection = false; // toggle history rejection, based on octree node hit ?
	bool visualizeHistoryRejection = false;

	bool interpolate_voxels = false;
	bool do_reprojection = false;

	float jiggle_factor = 1;

	int historyRejectionBufferDepth = 1; // Goes from 1 to 4, accept how many past nodes
	bool interpolate_alpha = false; // Accept even older values in buffer as perfect match or interpolate alpha between set value
									// and 1 depending on how far ago this node was hit
	
	bool visualize_edge_detection = false;
	bool visualize_active_alpha = false;
	bool visualize_motion_vectors = false;

	int Lod_feedback_level = 0;
	int max_lod_diff = 1;
	bool apply_lod_offset = false;
	bool visualize_feedback_level = false;

	int historyParentRejectionLevel = 0;	// 0 wrong value eventually, use higher nodes for history rejection ?
} TAA_params ;

typedef struct params_s {
	/* Window */
	int windowWidth = 1920;
	int windowHeight = 1080;

	int current_shader_output = 1;

	bool writeVideo = false;

	int voxel_count = 30;
	bool forceReload;
	bool renderVoxelsAABB = false;

	int min_visualization_depth = 0;
	int max_visualization_depth = 6;

	float jiggle_intensity = 0.0001;
	float diffuse_parameter = 0.2;

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

	long last_image_export = 0;
} Parameters;

extern Parameters parameters;

extern Camera_Params camera_params;

extern Octree_Params octree_params;

extern TAA_params taa_params;