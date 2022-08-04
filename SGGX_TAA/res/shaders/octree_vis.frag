#version 430 core

in vec4 world_pos;
flat in int depth;

uniform int output_type;

out vec4 out_color;

void main() {
    out_color = vec4(0);
    if (output_type == 0) {
        out_color = vec4(1);
    } else if (output_type == 1) {
        float depth_factor = (float(depth + 1) / 8);
        out_color = depth_factor * vec4(1);
    } else if (output_type == 2) {
        float depth_factor = 1 - (float(depth) / 8);
        out_color = depth_factor * vec4(1);
    } else if (output_type == 3) {
        out_color = vec4(abs(normalize(world_pos.xyz)), 1);
    }
}