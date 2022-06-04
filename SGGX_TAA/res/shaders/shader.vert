#version 430 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_tex_coords;
layout(location = 3) in vec4 vertex_color;

out vec4 interpolated_color;
out vec3 normal;
out vec4 world_pos;
out vec2 tex_coords;

uniform mat4 Model_Matrix;
uniform mat4 MVP_matrix;

void main() {
    gl_Position = MVP_matrix * position;
    interpolated_color = vertex_color;
    normal = (Model_Matrix * vec4(vertex_normal, 0)).xyz;
    world_pos = Model_Matrix * position;
    tex_coords = vertex_tex_coords;
}