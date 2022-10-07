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
uniform int visualize_motion_vectors;

uniform int visualize_active_alpha;
uniform int visualize_edge_detection;

uniform int history_buffer_depth;
uniform int interpolate_alpha;

uniform int do_history_parent;
uniform int do_reprojection;

in vec4 world_pos;

out vec4 out_color;

void main() {
    int horizontal_index = int(gl_FragCoord.x);
    int vertical_index = int(gl_FragCoord.y);
    int buffer_index = vertical_index * horizontal_pixels + horizontal_index;

    uint hit_leaf_index = imageLoad(node_hit_buffer, buffer_index).r;

    //TODO probably easiest to clear motion vectors here
    vec4 motion_vector = imageLoad(motion_vector_buffer, buffer_index);
    imageStore(motion_vector_buffer, buffer_index, vec4(0));
    float buffer_space_length = length(motion_vector.xy);
    float diff_3d_length = motion_vector.z;

    
    uvec4 buffer_val = imageLoad(rejection_buffer, buffer_index);
    
    float active_alpha = taa_alpha;

    float step_val = 0.01;
    int thinkIsEdge = diff_3d_length > step_val &&  hit_leaf_index == 0 ? 1 : 0; 

    if (thinkIsEdge != 0) {
        imageStore(space_hit_buffer, buffer_index, vec4(0));
    }
    
    
    int history_index = buffer_index;   

    if (do_reprojection > 0) {
        float motion_valid_factor = motion_vector.a;
        // active_alpha = motion_valid_factor * active_alpha + (1 - motion_valid_factor);

        float e = 2.71828182846;
        float denom = 1;
        float val = pow(e, -buffer_space_length / denom);
        //val = 1;
        active_alpha = active_alpha * val + (1 - val);

        active_alpha = sign(hit_leaf_index) * active_alpha + (1 - sign(hit_leaf_index));
        //out_color = vec4(buffer_space_length, buffer_space_length, buffer_space_length, 1);

        ivec2 coords = ivec2(gl_FragCoord.xy + motion_vector.xy);
        float buffer_move_factor = abs(motion_vector.a) > 1e-3 // make sure motion is large enough
            && coords.x < horizontal_pixels && coords.y < vertical_pixels // make sure result is in bounds
            ? 1 : 0;
        
        history_index = int(buffer_move_factor) * (coords.y * horizontal_pixels + coords.x) + (1 - int(buffer_move_factor)) * buffer_index;
    }    

    if (do_history_parent != 0) {
        uvec4 buffer_val = imageLoad(rejection_buffer, buffer_index);

        vec4 vis_color = vec4(0, 1, 0, 1);
        uvec4 new_buffer_val = uvec4(0);
        if (hit_leaf_index == 0) {
            active_alpha = thinkIsEdge + (1 - thinkIsEdge) * active_alpha;
            
            new_buffer_val = uvec4(buffer_val.r, 0, buffer_val.gb);
            uint factor = buffer_val.g;// + buffer_val.b + buffer_val.a;
            factor = factor > 0 ? 1 : 0;
            // active_alpha = (1 - factor) + factor * active_alpha;
        } else if (hit_leaf_index != buffer_val.r) {
            active_alpha = 1;
            vis_color = vec4(1, 0, 0, 1);
            new_buffer_val = uvec4(hit_leaf_index, buffer_val.gba);
        }

        new_buffer_val = thinkIsEdge * uvec4(0) + (1 - thinkIsEdge) * new_buffer_val;
        
        imageStore(rejection_buffer, buffer_index, new_buffer_val);

        if (active_alpha == 1) {
            imageStore(history_buffer, buffer_index, vec4(0));
        }
        if (visualize_history_rejection != 0) {
            out_color = vis_color;
            return;
        }
    }


    if (history_rejection_active != 0) {
        int rejection_index = buffer_index;
        rejection_index = history_index;

        uvec4 buffer_val = imageLoad(rejection_buffer, rejection_index);

        vec4 vis_color = vec4(0);
        uvec4 new_buffer_val = uvec4(0);
        if (hit_leaf_index == 0) {
            // vis_color = vec4(0, 0, 1, 1);
            active_alpha = thinkIsEdge + (1 - thinkIsEdge) * active_alpha;
            // new_buffer_val = buffer_val;
            new_buffer_val = uvec4(0, buffer_val.r, buffer_val.g, buffer_val.b);
            uint factor = buffer_val.r + buffer_val.g + buffer_val.b + buffer_val.a;
            factor = factor > 0 && buffer_space_length >= 0 ? 1 : 0;
            // active_alpha = factor + (1 - factor) * active_alpha;
        }
        else if (hit_leaf_index == buffer_val.r) {
            vis_color = vec4(0, 1, 0, 1);
            new_buffer_val = buffer_val;
        }
        else if (history_buffer_depth >= 2 && hit_leaf_index == buffer_val.g) {
            // vis_color = vec4(0.25, 0.75, 0, 1);
            vis_color = vec4(0, 1, 0, 1);
            active_alpha = interpolate_alpha * (0.25 + 0.75 * active_alpha) + (1 - interpolate_alpha) * active_alpha;
            new_buffer_val = uvec4(buffer_val.g, buffer_val.r, buffer_val.b, buffer_val.a);
        }
        else if (history_buffer_depth >= 3 && hit_leaf_index == buffer_val.b) {
            // vis_color = vec4(0.5, 0.5, 0, 1);
            vis_color = vec4(0, 1, 0, 1);
            active_alpha = interpolate_alpha * (0.5 + 0.5 * active_alpha) + (1 - interpolate_alpha) * active_alpha;
            new_buffer_val = uvec4(buffer_val.b, buffer_val.r, buffer_val.g, buffer_val.a);
        }
        else if (history_buffer_depth >= 4 && hit_leaf_index == buffer_val.a) {
            // vis_color = vec4(0.75, 0.25, 0, 1);
            vis_color = vec4(0, 1, 0, 1);
            active_alpha = interpolate_alpha * (0.75 + 0.25 * active_alpha) + (1 - interpolate_alpha) * active_alpha;
            new_buffer_val = uvec4(buffer_val.a, buffer_val.r, buffer_val.g, buffer_val.b);
        }
        else {
            // reject history
            vis_color = vec4(0, 0, 0, 1);
            active_alpha = 1.0;
            new_buffer_val = uvec4(hit_leaf_index, buffer_val.r, buffer_val.g, buffer_val.b);
        }

        new_buffer_val = thinkIsEdge * uvec4(0) + (1 - thinkIsEdge) * new_buffer_val;
        imageStore(rejection_buffer, buffer_index, new_buffer_val);
        if (visualize_history_rejection != 0) {
            out_color = vis_color;
            return;
        }
    }

    

    vec4 rendered_color = imageLoad(render_buffer, buffer_index);
    imageStore(render_buffer, buffer_index, vec4(0));
    vec4 old_color = imageLoad(history_buffer, history_index);
    
    vec4 new_buffer_color = rendered_color * active_alpha + old_color * (1 - active_alpha);
    imageStore(history_buffer, buffer_index, new_buffer_color);


    out_color = new_buffer_color;

    out_color = visualize_active_alpha * active_alpha + (1 - visualize_active_alpha) * out_color;

    out_color = /* vec4(0, 0, 0, 1) */ visualize_edge_detection * thinkIsEdge + (1 - visualize_edge_detection) * out_color;

    // out_color = vec4(thinkIsEdge) + rendered_color;

    out_color = visualize_motion_vectors * vec4(abs(motion_vector.xy), 0, 1) + (1 - visualize_motion_vectors) * out_color;
}