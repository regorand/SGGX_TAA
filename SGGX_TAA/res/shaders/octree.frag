#version 430 core

struct octree_node_s {
	uint type_and_index; 
    uint leaf_data_index;
};

struct inner_node_s {
	uint node_indices[8];
};

struct leaf_node_s {
	uint sigmas;
	uint rs;
    uint color;
};

#define NODE_TYPE(a) ((a & 0x80000000) != 0 ? 1 : 0)
#define NODE_INDEX(a) (a & 0x7FFFFFFF)

#define INNER_NODE_TYPE 0
#define LEAF_TYPE 1

#define READ_X_VALUE_MASK(a) (a & 0x000000FF)
#define READ_Y_VALUE_MASK(a) ((a >> 8) & 0x000000FF)
#define READ_Z_VALUE_MASK(a) ((a >> 16) & 0x000000FF)

// #define READ_Y_VALUE_MASK(a) ((a & 0x0000FF00) >> 8)
// #define READ_Z_VALUE_MASK(a) ((a & 0x00FF0000) >> 16)

#define READ_SPECIAL_VALUE_MASK(a) ((a & 0xFF000000) >> 24)

layout(std430, binding = 1) buffer octree_nodes
{
    octree_node_s nodes_data[];
};

layout(std430, binding = 2) buffer octree_inner_nodes
{
    inner_node_s inner_nodes_data[];
};

layout(std430, binding = 3) buffer octree_leaf_nodes
{
    leaf_node_s leaves_data[];
};

in vec4 world_pos;

out vec4 out_color;

uniform int nodes_size;
uniform int inner_nodes_size;
uniform int leaves_size;

uniform int max_tree_depth;

uniform int min_render_depth;
uniform int max_render_depth;
uniform float render_depth;

uniform int smooth_lod;

uniform int auto_lod;

uniform int roentgen_denom;

uniform float diffuse_parameter;
uniform int interpolate_voxels;

uniform int output_type;
uniform float AABBOutlineFactor;

uniform int LoD_feedback_types;
uniform int max_lod_diff;
uniform int apply_lod_offset;
uniform int visualize_feedback_level;

uniform float horizontal_pixel_size;
uniform float vertical_pixel_size;

uniform int horizontal_pixels;
uniform int vertical_pixels;

uniform int num_iterations;

// TAA Uniforms
uniform int do_history_parent;
uniform int history_parent_level;

// uniform samplerBuffer history_buffer;
layout(rgba32f) uniform imageBuffer render_buffer;
layout(rgba32f) uniform imageBuffer history_buffer;
layout(rgba32ui) uniform uimageBuffer rejection_buffer;
layout(r32ui) uniform uimageBuffer node_hit_buffer;
layout(rgba32f) uniform imageBuffer space_hit_buffer;
layout(rgba32f) uniform imageBuffer motion_vector_buffer;
layout(r32f) uniform imageBuffer lod_diff_buffer;

uniform vec3 lower;
uniform vec3 higher;
//uniform float AABBOutlineFactor;

uniform vec3 camera_pos;
uniform vec3 camera_view_dir;
uniform mat4 view_matrix;

uniform vec3 up_vector;

const vec3 light_dir = vec3(-1, 1, 0);

float step_0(float a) {
    return max(sign(a), 0);
}

vec3 step_0(vec3 a) {
    return max(sign(a), 0);
}

float clamped_dot(vec3 v1, vec3 v2) {
    return max(dot(v1, v2), 0);
}

float rand(vec2 co)
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

const uint max_stack_size = 32;
struct Stack {
    uint buf[max_stack_size];
    int current_size;
};

uint top(Stack stack) {
    return stack.buf[stack.current_size - 1];
}

uint pop(inout Stack stack) {
    stack.current_size--;
    stack.current_size = max(0, stack.current_size);
    return stack.buf[stack.current_size];
}

void push(inout Stack stack, uint value) {
    stack.buf[stack.current_size] = value;
    stack.current_size++;
    stack.current_size = min(31, stack.current_size);
}

bool hasCapacity(Stack stack) {
    return stack.current_size < max_stack_size;
}

bool hasElement(Stack stack) {
    return stack.current_size > 0;
}

// cherry picks an element from a stack by an index
uint cherry_pick(Stack stack, int index) {
    uint local_index = min(31, index);
    return stack.buf[local_index];
}

struct AABB {
    vec3 lower;
    vec3 upper;
};

struct AABBStack {
    AABB buf[max_stack_size];
    int current_size;
};

AABB top(AABBStack stack) {
    return stack.buf[stack.current_size - 1];
}

AABB pop(inout AABBStack stack) {
    stack.current_size--;
    stack.current_size = max(0, stack.current_size);
    return stack.buf[stack.current_size];
}

void push(inout AABBStack stack, AABB value) {
    stack.buf[stack.current_size] = value;
    stack.current_size++;    
    stack.current_size = min(31, stack.current_size);
}

bool hasCapacity(AABBStack stack) {
    return stack.current_size < max_stack_size;
}

bool hasElement(AABBStack stack) {
    return stack.current_size > 0;
}

struct Ray {
    vec3 ray_origin;
    vec3 ray_dir;
};

struct Intersection {
    float t_near;
    float t_far;
    int near_dimension;
    int far_dimension;
};

vec3 getAABBCenter(AABB aabb) {
    return (aabb.lower + aabb.upper) / 2;
}


Intersection getBoxIntersection(Ray ray, AABB aabb) {
    float t_near_tmp    = -100000;
    float t_far_tmp     =  100000;

    int near_dimension_index = -1;
    int far_dimension_index = -1;




    for (int i = 0; i < 3; i++) {
        float d = ray.ray_dir[i];
        // float div by 0 should give infinity, may need to make sure t1 is not always infinity
        // might not be a problem since in that case t_near is probably going to be farther than t_far meaning no intersection
        //if (abs(d) > 1e-6) {
        //}
            float t1 = (aabb.lower[i] - ray.ray_origin[i]) / d;
            float t2 = (aabb.upper[i] - ray.ray_origin[i]) / d;

            /*
            if (t1 > t2) {
                float tmp = t2;
                t2 = t1;
                t1 = tmp;
            }
            */
            float tmp = max(t1, t2);
            t1 = min(t1, t2);
            t2 = tmp;


            // Probably can do an if here, and save dimension index that gave current values in Intersection
            // That way we already know which dimension we exit again

            // 1 if t1 > t_near_tmp, 0 otherwise
            float t1_factor = step_0(t1 - t_near_tmp);
            float one_minus_t1_factor = 1 - t1_factor;
            t_near_tmp = t1_factor * t1 + one_minus_t1_factor * t_near_tmp;
            near_dimension_index = int(t1_factor * i + one_minus_t1_factor * near_dimension_index);
            
            float t2_factor = step_0(t_far_tmp - t2);
            float one_minus_t2_factor = 1 - t2_factor;
            t_far_tmp = t2_factor * t2 + one_minus_t2_factor * t_far_tmp;
            far_dimension_index = int(t2_factor * i + one_minus_t2_factor * far_dimension_index);

    }
    return Intersection(t_near_tmp, t_far_tmp, near_dimension_index, far_dimension_index);
}

bool isInBox(AABB box, vec3 pos) {
    return box.lower.x <= pos.x 
        && box.lower.y <= pos.y
        && box.lower.z <= pos.z
        && box.upper.x >= pos.x
        && box.upper.y >= pos.y
        && box.upper.z >= pos.z;
}

uint get_inner_index(AABB box, vec3 pos) {
    vec3 halfway = (box.upper + box.lower) / 2;
	uint x_fac = pos.x > halfway.x ? 1 : 0;
	uint y_fac = pos.y > halfway.y ? 1 : 0;
	uint z_fac = pos.z > halfway.z ? 1 : 0;

	return (x_fac << 2) + (y_fac << 1) + z_fac;
}

vec3 get_bound_factors(uint inner_index) {
    return vec3((inner_index & 0x4) >> 2, (inner_index & 0x2) >> 1, inner_index & 0x1);
}

void downScaleAABB(inout AABB aabb, uint inner_index) {
    vec3 halfway = (aabb.lower + aabb.upper) / 2;

    vec3 factors = get_bound_factors(inner_index);
    vec3 one_minus_factors = 1 - factors;

    aabb.upper = factors * aabb.upper + one_minus_factors * halfway;
	aabb.lower = one_minus_factors * aabb.lower + factors * halfway;
}

void upScaleAABB(inout AABB aabb, uint inner_index) {
    vec3 factors = get_bound_factors(inner_index);
    vec3 one_minus_factors = 1 - factors;
    vec3 new_lower = one_minus_factors * aabb.lower + factors * (2 * aabb.lower - aabb.upper);
    vec3 new_upper = factors * aabb.upper + one_minus_factors * (2 * aabb.upper - aabb.lower);
    aabb.lower = new_lower;
    aabb.upper = new_upper;
}

bool traverseDownToLeaf(vec3 position, 
    int max_depth,
    inout int current_depth,
    inout uint node_index,
    inout AABB running_aabb,
    inout Stack parent_stack,
    inout AABBStack aabb_stack) {
    // if (node_index >= nodes_size) { 
    //     return false;
    // }
    octree_node_s current_node = nodes_data[node_index];
    uint index = NODE_INDEX(current_node.type_and_index);
    uint type = NODE_TYPE(current_node.type_and_index);
        
    while (type == INNER_NODE_TYPE && current_depth < max_depth) {
        uint inner_index = get_inner_index(running_aabb, position);

        // if (!hasCapacity(parent_stack)) {
        //     return false;
        // }
        // if (!hasCapacity(aabb_stack)) {
        //     return false;
        // }
        current_depth++;
        push(parent_stack, node_index);
        push(aabb_stack, AABB(running_aabb.lower, running_aabb.upper));

        downScaleAABB(running_aabb, inner_index);

        // if (index >= inner_nodes_size) {
        //     return false;
        // }

        inner_node_s inner_node = inner_nodes_data[index];
        node_index = inner_node.node_indices[inner_index];

        // if (node_index >= nodes_size) {
        //     return false;
        // }
        octree_node_s current_node = nodes_data[node_index];
        index = NODE_INDEX(current_node.type_and_index);
        type = NODE_TYPE(current_node.type_and_index);
    }
    return true;
}

bool traverseUpToParent(vec3 position,
    inout int current_depth, 
    inout uint node_index,
    inout AABB running_aabb,
    inout Stack parent_stack,
    inout AABBStack aabb_stack) {
    while (!isInBox(running_aabb, position)) {
        if (!hasElement(parent_stack) || !hasElement(aabb_stack)) {
            return false;
        }

        node_index = pop(parent_stack);
        running_aabb = pop(aabb_stack);
        current_depth--;
    }
    return true;
}

mat3 buildSGGXMatrix(leaf_node_s leaf) {
    float sigma_x = float(READ_X_VALUE_MASK(leaf.sigmas)) / 255;
    float sigma_y = float(READ_Y_VALUE_MASK(leaf.sigmas)) / 255;
    float sigma_z = float(READ_Z_VALUE_MASK(leaf.sigmas)) / 255;

    float r_xy = (float(READ_X_VALUE_MASK(leaf.rs)) / 128) - 1;
    float r_xz = (float(READ_Y_VALUE_MASK(leaf.rs)) / 128) - 1;
    float r_yz = (float(READ_Z_VALUE_MASK(leaf.rs)) / 128) - 1;

    float Sxx = sigma_x * sigma_x;
    float Syy = sigma_y * sigma_y;
    float Szz = sigma_z * sigma_z;

    float Sxy = r_xy * sigma_x * sigma_y;
    float Sxz = r_xz * sigma_x * sigma_z;
    float Syz = r_yz * sigma_y * sigma_z;

	return mat3(Sxx, Sxy, Sxz, Sxy, Syy, Syz, Sxz, Syz, Szz);
}

void getSGGXFactors(leaf_node_s leaf, inout float Sxx, inout float Syy, inout float Szz, inout float Sxy, inout float Sxz, inout float Syz) {
        
    float sigma_x = float(READ_X_VALUE_MASK(leaf.sigmas)) / 255;
    float sigma_y = float(READ_Y_VALUE_MASK(leaf.sigmas)) / 255;
    float sigma_z = float(READ_Z_VALUE_MASK(leaf.sigmas)) / 255;

    float r_xy = (float(READ_X_VALUE_MASK(leaf.rs)) / 128) - 1;
    float r_xz = (float(READ_Y_VALUE_MASK(leaf.rs)) / 128) - 1;
    float r_yz = (float(READ_Z_VALUE_MASK(leaf.rs)) / 128) - 1;

    Sxx = sigma_x * sigma_x;
    Syy = sigma_y * sigma_y;
    Szz = sigma_z * sigma_z;
    
    Sxy = r_xy * sigma_x * sigma_y;
    Sxz = r_xz * sigma_x * sigma_z;
    Syz = r_yz * sigma_y * sigma_z;
}

void buildOrthonormalBasis(inout vec3 omega_1, inout vec3 omega_2, const vec3 omega_3)
{
    if(omega_3.z < -0.9999999f)
    {
        omega_1 = vec3 ( 0.0f , -1.0f , 0.0f );
        omega_2 = vec3 ( -1.0f , 0.0f , 0.0f );
    } else {
        const float a = 1.0f /(1.0f + omega_3.z );
        const float b = -omega_3.x*omega_3 .y*a ;
        omega_1 = vec3 (1.0f - omega_3.x*omega_3. x*a , b , -omega_3.x );
        omega_2 = vec3 (b , 1.0f - omega_3.y*omega_3.y*a , -omega_3.y );
    }
}

const float PI = 3.14159;

vec3 sample_VNDF(const vec3 wi,
const float S_xx, const float S_yy, const float S_zz,
const float S_xy, const float S_xz, const float S_yz,
const float U1, const float U2)
{
    // generate sample (u, v, w)
    const float r = sqrt(U1);
    const float phi = 2.0f*PI*U2;
    const float u = r*cos(phi);
    const float v= r*sin(phi);
    const float w = sqrt(1.0f - u*u - v*v);

    // build orthonormal basis
    vec3 wk, wj;
    buildOrthonormalBasis(wk, wj, wi);

    // project S in this basis
    const float S_kk = wk.x*wk.x*S_xx + wk.y*wk.y*S_yy + wk.z*wk.z*S_zz
    + 2.0f * (wk.x*wk.y*S_xy + wk.x*wk.z*S_xz + wk.y*wk.z*S_yz);
    const float S_jj = wj.x*wj.x*S_xx + wj.y*wj.y*S_yy + wj.z*wj.z*S_zz
    + 2.0f * (wj.x*wj.y*S_xy + wj.x*wj.z*S_xz + wj.y*wj.z*S_yz);
    const float S_ii = wi.x*wi.x*S_xx + wi.y*wi.y*S_yy + wi.z*wi.z*S_zz
    + 2.0f * (wi.x*wi.y*S_xy + wi.x*wi.z*S_xz + wi.y*wi.z*S_yz);
    const float S_kj = wk.x*wj.x*S_xx + wk.y*wj.y*S_yy + wk.z*wj.z*S_zz
    + (wk.x*wj.y + wk.y*wj.x)*S_xy
    + (wk.x*wj.z + wk.z*wj.x)*S_xz
    + (wk.y*wj.z + wk.z*wj.y)*S_yz;
    const float S_ki = wk.x*wi.x*S_xx + wk.y*wi.y*S_yy + wk.z*wi.z*S_zz
    + (wk.x*wi.y + wk.y*wi.x)*S_xy + (wk.x*wi.z + wk.z*wi.x)*S_xz + (wk.y*wi.z + wk.z*wi.y)*S_yz;
    const float S_ji = wj.x*wi.x*S_xx + wj.y*wi.y*S_yy + wj.z*wi.z*S_zz
    + (wj.x*wi.y + wj.y*wi.x)*S_xy
    + (wj.x*wi.z + wj.z*wi.x)*S_xz
    + (wj.y*wi.z + wj.z*wi.y)*S_yz;

    // compute normal
    float sqrtDetSkji = sqrt(abs(S_kk*S_jj*S_ii - S_kj*S_kj*S_ii - S_ki*S_ki*S_jj - S_ji*S_ji*S_kk + 2.0f*S_kj*S_ki*S_ji));
    float inv_sqrtS_ii = 1.0f / sqrt(S_ii);
    float tmp = sqrt(S_jj*S_ii-S_ji*S_ji);
    vec3 Mk = vec3(sqrtDetSkji / tmp, 0.0, 0.0);
    vec3 Mj = vec3(-inv_sqrtS_ii*(S_ki*S_ji-S_kj*S_ii)/tmp , inv_sqrtS_ii*tmp, 0);
    vec3 Mi = vec3(inv_sqrtS_ii*S_ki, inv_sqrtS_ii*S_ji, inv_sqrtS_ii*S_ii);
    vec3 wm_kji = normalize(u*Mk+v*Mj+w*Mi);

    // rotate back to world basis
    return wm_kji.x * wk + wm_kji.y * wj + wm_kji.z * wi;
}

vec3 sample_direction(int n, vec3 normal) {
    return normal;
}

vec3 evaluateSggx(leaf_node_s leaf, vec3 ray_dir, vec3 color) {

    vec3 wi = light_dir;
    vec3 wo = ray_dir;

    float S_xx = 0;
	float S_yy = 0;
	float S_zz = 0;

	float S_xy = 0;
	float S_xz = 0;
	float S_yz = 0;


    getSGGXFactors(leaf, S_xx, S_yy, S_zz, S_xy, S_xz, S_yz);

    float running_factor = 0;

    //int num_iterations = 128;

    for (int i = 0; i < num_iterations; i++) {
        float U1 = rand(gl_FragCoord.xy + i * world_pos.xy + 31 * min_render_depth * world_pos.yz + 33 * max_render_depth * world_pos.xz);
        float U2 = rand(gl_FragCoord.yz + U1 * world_pos.xy + 37 * min_render_depth * world_pos.yz + 41 * max_render_depth * world_pos.xz);

        const vec3 wm = sample_VNDF(wi, S_xx, S_yy, S_zz, S_xy, S_xz, S_yz, U1, U2);

        running_factor += (1.0f / PI) * max(0, dot(wo, wm));
    }

    running_factor /= num_iterations;

    return color * running_factor;
}

vec3 evaluateLeaf(leaf_node_s leaf, vec3 ray_dir) {
    vec3 sggx_color = vec3(1);

    uint r = READ_X_VALUE_MASK(leaf.color);
    uint g = READ_Y_VALUE_MASK(leaf.color);
    uint b = READ_Z_VALUE_MASK(leaf.color);

    int factor = int(r != 0 || g != 0 || b != 0);
    sggx_color = factor * vec3(float(r) / 255, float(g) / 255, float(b) / 255) + (1 - factor) * sggx_color;
    
    vec3 color = evaluateSggx(leaf, ray_dir, sggx_color);
    color *= (1 - diffuse_parameter);
    color += diffuse_parameter * sggx_color;
    return color;
}

vec3 evaluateNode(uint node_index, vec3 ray_dir, inout bool isNotEmpty, inout uint leaf_index) {
    leaf_index = nodes_data[node_index].leaf_data_index;

        leaf_node_s leaf = leaves_data[leaf_index];

        float density = float(READ_SPECIAL_VALUE_MASK(leaf.sigmas)) / 255;
        isNotEmpty = abs(density) > 1e-3;
        int factor = int(output_type == 0 && isNotEmpty);
        // if (factor == 1) {
        //     return evaluateLeaf(leaf, ray_dir);
        // }
        return factor * evaluateLeaf(leaf, ray_dir) + (1 - factor) * vec3(0);
        //if ()) {
        //}
    //if (leaf_index < leaves_size) {
    //} else {
    //    isEmpty = true;
    //    return vec3(0);
    //}
}


vec3 interpolateVoxelColor(vec3 ray_dir,
        vec3 ray_origin,
        Intersection base_intersection,
        vec3 voxel_color, 
        int max_depth,
        int current_depth,
        uint node_index,
        AABB running_aabb,
        Stack parent_stack,
        AABBStack aabb_stack) {

    vec3 tangent_view_plane = normalize(cross(ray_dir, up_vector));

    vec3 base_node_pos = ray_origin + ray_dir * ((base_intersection.t_far + base_intersection.t_near) / 2);

    Ray up_ray = Ray(base_node_pos, up_vector);
    Intersection up_intersect = getBoxIntersection(up_ray, running_aabb);

    Ray tangent_ray = Ray(base_node_pos, tangent_view_plane);
    Intersection tangent_intersect = getBoxIntersection(tangent_ray, running_aabb);

    vec3 vert_interp_dir = up_vector * sign(abs(up_intersect.t_near) - abs(up_intersect.t_far));
    vec3 horiz_interp_dir = tangent_view_plane * sign(abs(tangent_intersect.t_near) - abs(tangent_intersect.t_far));

    float eps = 1e-4;
    vec3 target_up = vec3(0);
    float up_factor = 0;
    if (abs(up_intersect.t_near) < abs(up_intersect.t_far)) {
        up_factor = abs(up_intersect.t_near) / abs(up_intersect.t_far);
        target_up = base_node_pos + (up_intersect.t_near - eps) * up_vector;
    } else {
        up_factor = abs(up_intersect.t_far)  / abs(up_intersect.t_near);
        target_up = base_node_pos + (up_intersect.t_far + eps) * up_vector;
    }

    vec3 target_tangent = vec3(0);
    vec3 tangent_offset = vec3(0);
    float tangent_factor = 0;
    if (abs(tangent_intersect.t_near) < abs(tangent_intersect.t_far)) {
        up_factor = abs(tangent_intersect.t_near) / abs(tangent_intersect.t_far);
        target_tangent = base_node_pos + (tangent_intersect.t_near - eps) * tangent_view_plane;
    } else {
        up_factor = abs(tangent_intersect.t_far)  / abs(tangent_intersect.t_near);
        tangent_offset = (tangent_intersect.t_far + eps) * tangent_view_plane;
        target_tangent = base_node_pos + tangent_offset;
    }

    vec3 target_up_tangent = target_up + tangent_offset;

    up_factor = (up_factor) / 2;
    tangent_factor = (tangent_factor) / 2;

    // get up color
    bool result = traverseUpToParent(target_up, current_depth, node_index, running_aabb, parent_stack, aabb_stack);
    if (!result) return voxel_color;

    result = traverseDownToLeaf(target_up, max_depth, current_depth, node_index, running_aabb, parent_stack, aabb_stack);
    if (!result) return voxel_color;

    bool dummy_bool;
    uint dummy_leaf_index = 0;
    vec3 up_color = evaluateNode(node_index, ray_dir, dummy_bool, dummy_leaf_index);

    // get up + tangent color
    result = traverseUpToParent(target_up_tangent, current_depth, node_index, running_aabb, parent_stack, aabb_stack);
    if (!result) return voxel_color;

    result = traverseDownToLeaf(target_up_tangent, max_depth, current_depth, node_index, running_aabb, parent_stack, aabb_stack);
    if (!result) return voxel_color;

    vec3 up_tangent_color = evaluateNode(node_index, ray_dir, dummy_bool, dummy_leaf_index);

    // get tangent color
    result = traverseUpToParent(target_up_tangent, current_depth, node_index, running_aabb, parent_stack, aabb_stack);
    if (!result) return voxel_color;

    result = traverseDownToLeaf(target_up_tangent, max_depth, current_depth, node_index, running_aabb, parent_stack, aabb_stack);
    if (!result) return voxel_color;

    vec3 tangent_color = evaluateNode(node_index, ray_dir, dummy_bool, dummy_leaf_index);

    vec3 interpolated_color_outside = up_factor * up_tangent_color + (1 - up_factor) * tangent_color;
    vec3 interpolated_color_inside = up_factor * up_color + (1 - up_factor) * voxel_color;

    return tangent_factor * interpolated_color_outside + (1 - tangent_factor) * interpolated_color_inside;
}

void main() {
    // General setup
    out_color = vec4(0);

    int horizontal_index = int(gl_FragCoord.x);
    int vertical_index = int(gl_FragCoord.y);
    int buffer_index = vertical_index * horizontal_pixels + horizontal_index;

    

    AABB octreeBound = AABB(lower, higher);

    vec3 ray_base = camera_pos;
    vec3 view_vector = world_pos.xyz - camera_pos;
    vec3 view_direction = normalize(view_vector);

    float eps = 1e-4;
    vec3 ray_eps = view_direction * eps;

    Ray pixel_ray = Ray(ray_base, view_direction);

    Intersection base_intersection = getBoxIntersection(pixel_ray, octreeBound);

    // LoD calculations
    int max_depth = int(ceil(render_depth));
    float smooth_lod_factor = fract(render_depth);  

    float thales_factor = base_intersection.t_near / length(view_vector);

    float LoD = render_depth;
    float target_leaf_size = vertical_pixel_size * thales_factor;
    float base_leaf_size = (higher.x - lower.x) / pow(2, max_tree_depth);

    // Auto LoD calculations
    if (auto_lod == 1) {
        if (target_leaf_size > base_leaf_size) {
            float render_offset = log2(target_leaf_size) - log2(base_leaf_size);

            LoD = max(1, LoD - render_offset);
        }
    }
    
    // LoD motion difference calculations
    float lod_buffer_value = imageLoad(lod_diff_buffer, buffer_index).r;
    int discrete_lod_buffer_value = min(max_lod_diff, int(round(lod_buffer_value)));

    max_depth = int(ceil(LoD));
    
    max_depth -= apply_lod_offset * discrete_lod_buffer_value;
    
    smooth_lod_factor = fract(LoD);

    if (max_depth > max_tree_depth) {
        smooth_lod_factor = 1;
    }

    // discard if not hitting AABB
    float base_factor = step_0(base_intersection.t_far - base_intersection.t_near);    
    if (base_factor == 0) {
        imageStore(space_hit_buffer, buffer_index, vec4(0));
        imageStore(node_hit_buffer, buffer_index, uvec4(0));
        imageStore(render_buffer, buffer_index, vec4(0));
        discard;
    }

    // traversal setup
    vec3 position = ray_base + base_intersection.t_near * view_direction + ray_eps;

    vec3 outline_color = (position - lower) / (higher - lower);

    vec3 sggx_sum_color = vec3(0);

    uint hit_leaf_index = 0;
    vec3 hit_center_loc = vec3(0);

    vec3 ray_dir = pixel_ray.ray_dir;

    uint parent_buf[max_stack_size];
    Stack parent_stack = Stack(parent_buf, 0);

    AABB aabb_stack_buf[max_stack_size];
    AABBStack aabb_stack = AABBStack(aabb_stack_buf, 0);

    AABB running_aabb = AABB(octreeBound.lower, octreeBound.upper);

    uint node_index = 0;
    int current_depth = 0;

    // Start traversal
    bool result = traverseDownToLeaf(position, max_depth, current_depth, node_index, running_aabb, parent_stack, aabb_stack);
    Intersection leaf_intersection = getBoxIntersection(pixel_ray, running_aabb);

    float count = 0;

    while (result && count < 200) {
        // Render Leaf
        count++;
    
        bool isNotEmpty = false;
        uint leaf_index = 0;
        vec3 color = evaluateNode(node_index, ray_dir, isNotEmpty, leaf_index);

        if (isNotEmpty) {
            if (smooth_lod != 0 && parent_stack.current_size > 0) {
                // smooth lod still a little broken since LoD slider change
                uint parent_node = top(parent_stack);

                bool parent_empty = false;
                uint parent_leaf = 0;
                vec3 parent_color = evaluateNode(parent_node, ray_dir, parent_empty, parent_leaf);
                
                color = smooth_lod_factor * color + (1 - smooth_lod_factor) * parent_color;
            }

            sggx_sum_color += color;
            hit_leaf_index = leaf_index;
            hit_center_loc = (running_aabb.lower + running_aabb.upper) / 2;
            break;
        }

        // Get new Position
        position = ray_base + leaf_intersection.t_far * ray_dir + ray_eps;

        // Get smallest common parent of current leaf and leaf in which the new position is located
        result = traverseUpToParent(position, current_depth, node_index, running_aabb, parent_stack, aabb_stack);
        if (!result) break;

        // get Next Leaf
        result = traverseDownToLeaf(position, max_depth, current_depth, node_index, running_aabb, parent_stack, aabb_stack);
        if (!result) break;

        // Find leaf Intersection
        leaf_intersection = getBoxIntersection(pixel_ray, running_aabb);
    }

    // Calculate output Color after traversal 
    if (output_type == 0) {
        out_color = vec4(sggx_sum_color, step_0(hit_leaf_index) * 1);
    }

    // This renders an outline around the bounding box if AABBOutlineFactor uniform is set to 1
    out_color += AABBOutlineFactor * 0.3 * vec4(outline_color, 1);


    // From here: TAA 
    if (do_history_parent != 0 && history_parent_level > 0) {
        int stack_node_index = max(parent_stack.current_size - history_parent_level, 0);
        hit_leaf_index = cherry_pick(parent_stack, stack_node_index);
    }

    uint previous_hit_leaf_index = imageLoad(node_hit_buffer, buffer_index).r;
    imageStore(node_hit_buffer, buffer_index, uvec4(0));
    imageStore(render_buffer, buffer_index, vec4(0));
    
    vec4 previous_hit = imageLoad(space_hit_buffer, buffer_index);

    imageStore(space_hit_buffer, buffer_index, vec4(position, 1));
    
    vec4 h_previous_hit = vec4(previous_hit.xyz, 1);
    vec4 h_current_hit = vec4(position, 1);

    vec4 cam_space_prev = view_matrix * h_previous_hit;
    vec4 cam_space_curr = view_matrix * h_current_hit;

    float z_diff = cam_space_prev.z - cam_space_curr.z;

    vec4 cam_space_prev_flat = vec4(cam_space_prev.xy / cam_space_prev.z, 1, 1);
    vec4 cam_space_curr_flat = vec4(cam_space_curr.xy / cam_space_curr.z, 1, 1);

    vec2 flat_space_diff = cam_space_prev_flat.xy - cam_space_curr_flat.xy;
    
    vec2 buffer_space_motion = flat_space_diff / vec2(horizontal_pixel_size, vertical_pixel_size);

    ivec2 estimated_history_origin = ivec2(gl_FragCoord.xy + buffer_space_motion);

    buffer_space_motion *= sign(hit_leaf_index);

    float diff_3d_length = length(previous_hit.xyz - position);

    

    float offset = 0;
    if (LoD_feedback_types == 1) {
        float buffer_space_dist = length(buffer_space_motion.xy);

        offset = max(0, log2(buffer_space_dist));
    } else if (LoD_feedback_types == 2) {
        float f_b = diff_3d_length / base_leaf_size;

        offset = max(0, log2(f_b));
    } else if (LoD_feedback_types == 3) {
        float previous_voxel_size = (higher.x - lower.x) / pow(2, max_depth);
        float f_p = diff_3d_length / previous_voxel_size;

        offset = max(0, log2(f_p));
    } else if (LoD_feedback_types == 4) {
        float previous_voxel_size = (higher.x - lower.x) / pow(2, max_depth);
        float f_p = diff_3d_length / previous_voxel_size;

        offset = log2(f_p);
    }

    offset *= step_0(hit_leaf_index);//  * step_0(previous_hit_leaf_index);
    if (previous_hit_leaf_index == 0) {
        offset = 1;
    }

    if (visualize_feedback_level != 0) {
        out_color = vec4(offset / 20, 0, 0, 1);
        if (offset == 0) {
           out_color = vec4(0);
        }
    } else {
        imageStore(lod_diff_buffer, buffer_index, vec4(offset, 0, 0, 0));
    }

    int valid_motion_factor =   hit_leaf_index != 0 && estimated_history_origin.x < horizontal_pixels 
                                && estimated_history_origin.y < vertical_pixels
                                    ? 1 : 0;

    vec4 motion_buffer_content = vec4(buffer_space_motion.xy, diff_3d_length, valid_motion_factor);

    int origin_buffer_index =   valid_motion_factor * (estimated_history_origin.y * horizontal_pixels + estimated_history_origin.x)
                                + (1 - valid_motion_factor) * buffer_index;

    imageStore(motion_vector_buffer, origin_buffer_index, motion_buffer_content);

    imageStore(node_hit_buffer, buffer_index, uvec4(hit_leaf_index));

    imageStore(render_buffer, buffer_index, out_color);
}