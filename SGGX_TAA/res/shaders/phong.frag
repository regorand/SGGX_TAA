#version 430 core

#define MAX_LIGHTS 128

in vec4 interpolated_color;
in vec4 world_pos;
in vec3 normal;
in vec2 tex_coords;

uniform int num_lights;
uniform vec3 light_positions[MAX_LIGHTS];
uniform vec3 lights_intensities[MAX_LIGHTS];

uniform vec3 K_A;
uniform vec3 K_D;
uniform vec3 K_S;
uniform vec3 camera_pos;

uniform int output_type;

uniform float spec_exponent;
uniform sampler2D diff_texture;

out vec4 out_color;

float pos_dot(vec3 light_direction, vec3 normal) {
    return max(dot(light_direction, normal), 0);
}

vec3 getReflectionDirection(vec3 light_direction, vec3 normal) {
    return 2 * pos_dot(light_direction, normal) * normal - light_direction;
}

vec4 shade_phong(vec4 diffuse, vec3 normal, vec3 view_direction) {
    vec4 color = vec4(K_A, 1) * diffuse;
    
    for (int i = 0; i < num_lights; i++) {
        vec3 light_dir = normalize(light_positions[i] - world_pos.xyz);

        //float light_normal_dot = dot(light_direction, normalized_normal);
        float light_normal_dot = dot(light_dir, normal);
        light_normal_dot = max(light_normal_dot, 0);

        // remove conditional
        if (light_normal_dot > 0) {
            //float lighting_dot = pos_dot(light_direction, normalized_normal);
            color += diffuse * light_normal_dot * vec4(lights_intensities[i], 1);

            // use reflect function
            //vec3 reflection_direct = 2 * light_normal_dot * normal - light_direction;
            vec3 reflection_direct = reflect(-light_dir, normal);
            color += vec4(K_S, 1) * vec4(lights_intensities[i], 1) * pow(pos_dot(view_direction, reflection_direct), spec_exponent);
        }
    }
    return color;
}

void main() {
    vec4 diffuse = texture(diff_texture, tex_coords);
    vec4 lightIntensity = vec4(1, 1, 1, 1);
    vec3 normalized_normal = normalize(normal);
    vec3 view_direction = normalize(camera_pos - world_pos.xyz);
    
    out_color = vec4(0, 0, 0, 1);

    if (output_type == 0) {
        out_color += vec4(K_A, 1);
    } else if (output_type == 1) {
        out_color += shade_phong(diffuse, normalized_normal, view_direction);
    } else if (output_type == 2) {
        out_color += shade_phong(diffuse, normalized_normal, view_direction);        
    } else if (output_type == 3) {
        out_color = vec4(normalized_normal, 1);
    } else if (output_type == 4) {
        out_color = vec4(abs(normalized_normal), 1);
    } else if (output_type == 5) {
        out_color = vec4(world_pos.xyz / world_pos.w, 1);
    } else if (output_type == 6) {
        //out_color = factor * vec4(voxelIndex / dimension, 1);
        //out_color = vec4(vec3(density), 1);
    }
    
}