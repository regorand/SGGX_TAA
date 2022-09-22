#version 430 core

// uniform samplerBuffer history_buffer;
layout(rgba32f) uniform imageBuffer render_buffer;
layout(rgba32f) uniform imageBuffer history_buffer;
layout(rgba32ui) uniform uimageBuffer rejection_buffer;
layout(r32ui) uniform uimageBuffer node_hit_buffer;
layout(rgba32f) uniform imageBuffer space_hit_buffer;
layout(rgba32f) uniform imageBuffer motion_vector_buffer;
layout(r32f) uniform imageBuffer lod_diff_buffer;


uniform int horizontal_pixels;
uniform int vertical_pixels;

uniform float taa_alpha;

uniform int history_rejection_active;
uniform int visualize_history_rejection;

uniform int history_buffer_depth;
uniform int interpolate_alpha;
uniform int history_parent_level;

uniform int do_reprojection;

in vec4 world_pos;

out vec4 out_color;

void main() {
    int horizontal_index = int(gl_FragCoord.x);
    int vertical_index = int(gl_FragCoord.y);
    int buffer_index = vertical_index * horizontal_pixels + horizontal_index;

    uint hit_leaf_index = imageLoad(node_hit_buffer, buffer_index).r;

    //TODO probably easiest to clear motion vectors here
    vec3 motion_vector = imageLoad(motion_vector_buffer, buffer_index).xyz;
    imageStore(motion_vector_buffer, buffer_index, vec4(0));

    float step_val = 0;
    // TODO: make this factor smooth ?
    int thinkIsEdge = length(motion_vector) > step_val ? 1 : 0;
    
    float active_alpha = taa_alpha;
    if (history_rejection_active != 0) {
        uvec4 buffer_val = imageLoad(rejection_buffer, buffer_index);
            
        vec4 vis_color = vec4(0);
        uvec4 store_color = uvec4(0);
        if (hit_leaf_index == 0) {
            vis_color = vec4(0, 0, 1, 1);
            active_alpha = thinkIsEdge + (1 - thinkIsEdge) * active_alpha;
            store_color = thinkIsEdge * uvec4(0) + (1 - thinkIsEdge) * buffer_val;
        }
        else if (hit_leaf_index == buffer_val.r) {
            vis_color = vec4(0, 1, 0, 1);
            store_color = buffer_val;
        }
        else if (history_buffer_depth >= 2 && hit_leaf_index == buffer_val.g) {
            vis_color = vec4(0.25, 0.75, 0, 1);
            active_alpha = interpolate_alpha * (0.25 + 0.75 * active_alpha) + (1 - interpolate_alpha) * active_alpha;
            store_color = uvec4(buffer_val.g, buffer_val.r, buffer_val.b, buffer_val.a);
            // imageStore(rejection_buffer, buffer_index, uvec4(buffer_val.g, buffer_val.r, buffer_val.b, buffer_val.a));
        }
        else if (history_buffer_depth >= 3 && hit_leaf_index == buffer_val.b) {
            vis_color = vec4(0.5, 0.5, 0, 1);
            active_alpha = interpolate_alpha * (0.5 + 0.5 * active_alpha) + (1 - interpolate_alpha) * active_alpha;
            store_color = uvec4(buffer_val.b, buffer_val.r, buffer_val.g, buffer_val.a);
            // imageStore(rejection_buffer, buffer_index, );
        }
        else if (history_buffer_depth >= 4 && hit_leaf_index == buffer_val.a) {
            vis_color = vec4(0.75, 0.25, 0, 1);
            active_alpha = interpolate_alpha * (0.75 + 0.25 * active_alpha) + (1 - interpolate_alpha) * active_alpha;
            store_color = uvec4(buffer_val.a, buffer_val.r, buffer_val.g, buffer_val.b);
            // imageStore(rejection_buffer, buffer_index, );
        }
        else {
            // reject history
            vis_color = vec4(1, 0, 0, 1);
            active_alpha = 1.0;
            // imageStore(rejection_buffer, buffer_index, uvec4(hit_leaf_index, buffer_val.r, buffer_val.g, buffer_val.b));
            store_color = uvec4(hit_leaf_index, buffer_val.r, buffer_val.g, buffer_val.b);
        }
        imageStore(rejection_buffer, buffer_index, store_color);
        if (visualize_history_rejection != 0) {
            out_color = vis_color;
            return;
        }
    }

    int history_index = buffer_index;
    
    float l =  length(motion_vector.xy);
    bool motion_vector_above_half = motion_vector.x >= 0.5 || motion_vector.y >= 0.5;
    if (do_reprojection != 0 && abs(motion_vector.z) > 1e-3 && l  < 3) {
        ivec2 coords = ivec2(gl_FragCoord.xy + motion_vector.xy);
        if (coords.x < horizontal_pixels && coords.y < vertical_pixels) {
            history_index = coords.y * horizontal_pixels + coords.x;
        }
    }


    float motion_vector_length = length(motion_vector.xy);
    //imageStore(lod_diff_buffer, buffer_index, vec4(motion_vector_length, 0, 0, 0));
    if (false || motion_vector_above_half) {
        imageStore(lod_diff_buffer, buffer_index, vec4(1, 0, 0, 0));
        out_color = vec4(1, 0, 0, 1);
        // return;
    }
    

    vec4 rendered_color = imageLoad(render_buffer, buffer_index);
    vec4 old_color = imageLoad(history_buffer, history_index);

    vec4 new_buffer_color = rendered_color * active_alpha + old_color * (1 - active_alpha);
    imageStore(history_buffer, buffer_index, new_buffer_color);
    imageStore(motion_vector_buffer, buffer_index, vec4(0));

    int mvf = 1;

    out_color = vec4((motion_vector.xyz) / mvf, 1);
    //out_color = vec4(vec3(motion_vector_length, motion_vector_length, motion_vector_length) / mvf, 1) ;


    out_color = new_buffer_color;

    // out_color = vec4(motion_vector.xy, 0, 1);
    // out_color = vec4(active_alpha, 0, 0, 1);
}