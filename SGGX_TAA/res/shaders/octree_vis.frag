#version 430 core

in vec4 world_pos;
flat in int depth;

out vec4 out_color;

void main() {
    out_color = vec4(0);
    //float depth_factor = 1 - (float(depth) / 8);
    float depth_factor = (float(depth + 1) / 8);
    out_color = depth_factor * vec4(1);
}