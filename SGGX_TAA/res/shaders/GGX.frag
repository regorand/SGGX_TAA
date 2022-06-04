#version 430 core

in vec4 interpolated_color;
in vec4 world_pos;
in vec3 normal;
in vec2 tex_coords;


// phong parameters
uniform vec3 K_A;
uniform vec3 K_D;
uniform vec3 K_S;
uniform float spec_exponent;

// geometry and locations
uniform vec3 camera_pos;

// lights
#define MAX_LIGHTS 128

uniform int num_lights;
uniform vec3 light_positions[MAX_LIGHTS];
uniform vec3 lights_intensities[MAX_LIGHTS];

// Textures
uniform sampler2D diff_texture;

// GGX
uniform float ggx_parameter;
uniform int samples;

out vec4 out_color;

const float PI = 3.14159265359;

float chi_plus(float a) {
    return max(sign(a), 0);
}

float rand(vec2 co)
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float normals_distribution(vec3 micro_normal, vec3 macro_normal) {
    float ggx_param_squared = ggx_parameter * ggx_parameter;
    float cos_theta_m = dot(micro_normal, macro_normal);
    float sin_theta_m = length(cross(micro_normal, macro_normal));
    if (cos_theta_m == 0) {
        return 1;
    }
    float tan_theta_mk = sin_theta_m / cos_theta_m;
    return (ggx_param_squared * chi_plus(dot(micro_normal, macro_normal))) / (PI * pow(abs(cos_theta_m), 4) * pow(abs(ggx_param_squared + pow(abs(tan_theta_mk), 2)), 2));
}

float G1_distribution(vec3 v, vec3 micro_normal, vec3 macro_normal) {
    float v_dot_m = dot(v, micro_normal);
    float cos_theta_n = dot(v, macro_normal);
    float sin_theta_n = length(cross(v, macro_normal));
    float tan_theta_n = sin_theta_n / cos_theta_n;

    return chi_plus(v_dot_m / cos_theta_n) * 2 / (1 + sqrt(1 + ggx_parameter * ggx_parameter * tan_theta_n * tan_theta_n));
}

float G(vec3 i, vec3 o, vec3 macro_normal, vec3 h_r) {
    return G1_distribution(i, h_r, macro_normal) * G1_distribution(o, h_r, macro_normal);
}

vec2 generateSample(int n) {
    float xi1 = rand(vec2(world_pos.x + world_pos.z * n * 31, world_pos.y + world_pos.x * n * 37));
    float xi2 = rand(vec2(world_pos.y + world_pos.z * n * 39, world_pos.x + n * world_pos.y * n * 41 + world_pos.z * n * 43));

    float theta_m = atan(ggx_parameter * sqrt(xi1) / sqrt(1 - xi1));
    float psi_m = 2 * PI * xi2;
    return vec2(theta_m, psi_m);
}

float pos_dot(vec3 light_direction, vec3 normal) {
    return max(dot(light_direction, normal), 0);
}

void main() {
    vec3 macro_normal = normalize(normal);
    vec3 tangent = vec3(0);
    if (macro_normal.z == 0) {
        tangent = vec3(-macro_normal.y, macro_normal.x, 0);
    } else {
        tangent = normalize(vec3(1, 1, -(macro_normal.x + macro_normal.y) / macro_normal.z));
    }
    vec3 surface_color = texture(diff_texture, tex_coords).xyz;
    float diffuseCoeff = 0.2;
    vec3 color = surface_color * diffuseCoeff;

    vec3 view_direction = normalize(camera_pos - world_pos.xyz);

    for (int i = 0; i < num_lights; i++) {
        vec3 light_dir = normalize(light_positions[i] - world_pos.xyz);
        
        float light_normal_dot = dot(light_dir, macro_normal);
        float factor = chi_plus(light_normal_dot);

        vec3 h_r_arrow = light_normal_dot * (light_dir + view_direction);
        vec3 h_r = normalize(h_r_arrow);

        float jacobian = 1 / (4 * sign(dot(view_direction, h_r)));

        // TODO: consider distance to light
        // I know GGX gets dark, reason you can't see anything because 
        color += factor * jacobian * normals_distribution(h_r, macro_normal) * G(light_dir, view_direction, macro_normal, h_r) * lights_intensities[i] * surface_color;
    }
    

    out_color = vec4(color, 1);
}