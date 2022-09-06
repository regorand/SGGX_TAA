#version 430 core

layout(location = 0) in vec4 position;

out vec4 world_pos;

uniform vec2 jiggle_offset;

void main() {
    vec4[] pos = {
        vec4(-1.0, -1.0, 0, 1),
        vec4( 1.0, -1.0, 0, 1),
        vec4(-1.0,  1.0, 0, 1),
        vec4( 1.0,  1.0, 0, 1)
    };

    gl_Position = pos[gl_VertexID] + vec4(jiggle_offset.xy, 0, 0);
    world_pos = position;
    //world_pos = pos[gl_VertexID];
}