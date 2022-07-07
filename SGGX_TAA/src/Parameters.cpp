#include "Parameters.h"

Parameters parameters;

void initParams() 
{
	parameters.active_shader_output = shader_output_types[0];
	parameters.active_render_type = render_types[0];
}
