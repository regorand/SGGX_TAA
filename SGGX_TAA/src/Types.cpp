#include "Types.h"

std::vector<render_type> render_types;
std::vector<std::string> render_types_names;

void initTypes() 
{
	std::vector<std::string> default_shader_outputs({ "White", "Shaded", "Shaded Solid", "Normal", "Abs Normal", "Position", "Custom" });
	render_types.push_back({ "Rasterization" , default_shader_outputs });
	render_types.push_back({ "Voxels" , default_shader_outputs });
	
	std::vector<std::string> octree_vis_shader_outputs({ "White", "Bright leafs", "Bright Root", "Custom" });
	render_types.push_back({ "Octree Visualization" , octree_vis_shader_outputs });

	std::vector<std::string> octree_shader_outputs({ "SGGX", "Roentgen", "Custom"});
	render_types.push_back({ "Octree" , octree_shader_outputs });

	std::transform(render_types.begin(),
		render_types.end(),
		std::back_inserter(render_types_names),
		[](render_type r) {
			return r.name;
		});
}

std::vector<std::string> render_types_2({ "Rasterization", "Voxels", "Octree Visualization", "Octree"});

std::vector<std::string> shader_output_types({ "white", "shaded", "shaded solid", "normal", "abs_normal", "position", "custom" });
