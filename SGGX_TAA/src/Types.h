#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>

constexpr auto RASTERIZATION_RENDER_INDEX = 0;
constexpr auto VOXEL_INDEX_RENDER_INDEX = 1;
constexpr auto OCTREE_VIS_RENDER_INDEX = 2;
constexpr auto OCTREE_RENDER_INDEX = 3;

typedef struct {
	std::string name;
	std::vector<std::string> shader_output_types;
	int current_render_type;
} render_type;

void initTypes();

extern std::vector<render_type> render_types;
extern std::vector<std::string> render_types_names;

extern std::vector<std::string> render_types_2;

extern std::vector<std::string> shader_output_types;

//TODO specifiy sensible default here and use in parameters
