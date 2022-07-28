#version 430 core

layout(location = 0) in vec4 position;
layout(location = 1) in int vertex_depth;

out vec4 world_pos;
flat out int depth;
uniform mat4 Model_Matrix;
uniform mat4 MVP_matrix;

void main() {
    gl_Position = MVP_matrix * position;
    world_pos = Model_Matrix * position;
    depth = vertex_depth;
}