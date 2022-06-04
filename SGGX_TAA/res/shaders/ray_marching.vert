#version 430 core

layout(location = 0) in vec4 position;

out vec4 world_pos;

void main() {
    vec4[] pos = {
        vec4(-1.0, -1.0, 0, 1),
        vec4( 1.0, -1.0, 0, 1),
        vec4(-1.0,  1.0, 0, 1),
        vec4( 1.0,  1.0, 0, 1)
    };

    gl_Position = pos[gl_VertexID];
    world_pos = position;
    //world_pos = pos[gl_VertexID];
}