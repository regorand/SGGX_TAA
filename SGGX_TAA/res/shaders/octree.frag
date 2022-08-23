#version 430 core

struct octree_node_s {
	uint type_and_index; 
    uint leaf_data_index;
};

struct inner_node_s {
	uint node_indices[8];
};

struct leaf_node_s {
	float value;

/*
    float normal_x;
    float normal_y;
    float normal_z;
*/

    float sigma_x;
	float sigma_y;
	float sigma_z;
    
	float r_xy;
	float r_xz;
	float r_yz;
};

#define NODE_TYPE(a) ((a & 0x80000000) != 0 ? 1 : 0)
#define NODE_INDEX(a) (a & 0x7FFFFFFF)

#define INNER_NODE_TYPE 0
#define LEAF_TYPE 1

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

uniform int auto_lod;

uniform int roentgen_denom;

uniform int output_type;
uniform float AABBOutlineFactor;

uniform float horizontal_pixel_size;
uniform float vertical_pixel_size;

uniform int horizontal_pixels;
uniform int vertical_pixels;

uniform int num_iterations;

// uniform samplerBuffer history_buffer;
layout(rgba32f) uniform imageBuffer history_buffer;

uniform vec3 lower;
uniform vec3 higher;
//uniform float AABBOutlineFactor;

uniform vec3 camera_pos;

const vec3 light_dir = vec3(-1, 1, 0);

float step_0(float a) {
    return max(sign(a), 0);
}

vec3 step_0(vec3 a) {
    return max(sign(a), 0);
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
    if (stack.current_size > 0) {
        return stack.buf[stack.current_size - 1];
    }
    return 0;
}

uint pop(inout Stack stack) {
    if (stack.current_size > 0) {
        stack.current_size--;
        return stack.buf[stack.current_size];
    }
    return 0;
}

void push(inout Stack stack, uint value) {
    if (stack.current_size < max_stack_size) {
        stack.buf[stack.current_size] = value;
        stack.current_size++;
    }
}

bool hasCapacity(Stack stack) {
    return stack.current_size < max_stack_size;
}

bool hasElement(Stack stack) {
    return stack.current_size > 0;
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
    if (stack.current_size > 0) {
        return stack.buf[stack.current_size - 1];
    }
    return AABB(vec3(0), vec3(1));
}

AABB pop(inout AABBStack stack) {
    if (stack.current_size > 0) {
        stack.current_size--;
        return stack.buf[stack.current_size];
    }
    return AABB(vec3(0), vec3(1));
}

void push(inout AABBStack stack, AABB value) {
    if (stack.current_size < max_stack_size) {
        stack.buf[stack.current_size] = value;
        stack.current_size++;
    }
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
    float t_near_tmp = -1;
    float t_far_tmp = 100000;

    int near_dimension_index = -1;
    int far_dimension_index = -1;

    for (int i = 0; i < 3; i++) {
        float d = ray.ray_dir[i];
        if (abs(d) > 1e-6) {
            float t1 = (aabb.lower[i] - ray.ray_origin[i]) / d;
            float t2 = (aabb.upper[i] - ray.ray_origin[i]) / d;
            if (t1 > t2) {
                float tmp = t2;
                t2 = t1;
                t1 = tmp;
            }
            // Probably can do an if here, and save dimension index that gave current values in Intersection
            // That way we already know which dimension we exit again
            if (t1 > t_near_tmp) {
                t_near_tmp  = t1;
                near_dimension_index = i;
            }
            if (t2 < t_far_tmp) {
                t_far_tmp = t2;
                far_dimension_index = i;
            }
            //t_near_tmp = max(t_near_tmp, t1);
            //t_far_tmp = min(t_far_tmp, t2);
        }
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

/*
void get_parent_AABB(inout AABB box, uint inner_index) {
    vec3 factors = get_bound_factors(inner_index);
    vec3 one_minus_factors = 1 - factors;
    vec3 new_lower = one_minus_factors * box.lower + factors * (2 * box.lower - box.upper);
    vec3 new_upper = factors * box.upper + one_minus_factors * (2 * box.upper - box.lower);
    box.lower = new_lower;
    box.upper = new_upper;
}
*/

bool traverseDownToLeaf(vec3 position, 
    int max_depth,
    inout int current_depth,
    inout uint node_index,
    inout AABB running_aabb,
    inout Stack parent_stack,
    inout AABBStack aabb_stack) {
    if (node_index >= nodes_size) { 
        return false;
    }
    octree_node_s current_node = nodes_data[node_index];
    uint index = NODE_INDEX(current_node.type_and_index);
    uint type = NODE_TYPE(current_node.type_and_index);
        
    while (type == INNER_NODE_TYPE && current_depth < max_depth) {
        uint inner_index = get_inner_index(running_aabb, position);

        if (!hasCapacity(parent_stack)) {
            return false;
        }
        if (!hasCapacity(aabb_stack)) {
            return false;
        }
        current_depth++;
        push(parent_stack, node_index);
        push(aabb_stack, AABB(running_aabb.lower, running_aabb.upper));

        downScaleAABB(running_aabb, inner_index);

        if (index >= inner_nodes_size) {
            return false;
        }

        inner_node_s inner_node = inner_nodes_data[index];
        node_index = inner_node.node_indices[inner_index];

        if (node_index >= nodes_size) {
            return false;
        }
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
    float Sxx = leaf.sigma_x * leaf.sigma_x;
	float Syy = leaf.sigma_y * leaf.sigma_y;
	float Szz = leaf.sigma_z * leaf.sigma_z;
	
	float Sxy = leaf.r_xy * leaf.sigma_x * leaf.sigma_y;
	float Sxz = leaf.r_xz * leaf.sigma_x * leaf.sigma_z;
	float Syz = leaf.r_yz * leaf.sigma_y * leaf.sigma_z;

	return mat3(Sxx, Sxy, Sxz, Sxy, Syy, Syz, Sxz, Syz, Szz);
}

void getSGGXFactors(leaf_node_s leaf, inout float Sxx, inout float Syy, inout float Szz, inout float Sxy, inout float Sxz, inout float Syz) {
    Sxx = leaf.sigma_x * leaf.sigma_x;
	Syy = leaf.sigma_y * leaf.sigma_y;
	Szz = leaf.sigma_z * leaf.sigma_z;

	Sxy = leaf.r_xy * leaf.sigma_x * leaf.sigma_y;
	Sxz = leaf.r_xz * leaf.sigma_x * leaf.sigma_z;
	Syz = leaf.r_yz * leaf.sigma_y * leaf.sigma_z;
}


vec4 doSGGX() {
    return vec4(1);
}

float clamped_dot(vec3 v1, vec3 v2) {
    return max(dot(v1, v2), 0);
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

vec3 evaluateSggx(leaf_node_s leaf, vec3 ray_dir) {

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

    return vec3(1) * running_factor;

    /*
    int num_point_samples = 1;
    vec3 out_dir = - ray_dir;

    float sum = 0;
    for (int i = 0; i < num_point_samples; i++) {
        sum += clamped_dot(sample_direction(1, normal), out_dir);
    }
    sum /= PI * num_point_samples;

    //mat3 sggx_matrix = buildSGGXMatrix(leaf);
    return sum * vec3(1);
    */
}

void main() {
    

    out_color = vec4(0);

    if (output_type == 2) {
        int horizontal_index = int(gl_FragCoord.x);
        int vertical_index = int(gl_FragCoord.y);

        int buffer_index = vertical_index * horizontal_pixels + horizontal_index;

        out_color = imageLoad(history_buffer, buffer_index);
        return;
    }

    AABB octreeBound = AABB(lower, higher);
    int max_depth = max_render_depth;

    vec3 ray_base = camera_pos;
    vec3 view_direction = normalize(world_pos.xyz - camera_pos);

    float eps = 1e-4;
    vec3 ray_eps = view_direction * eps;

    Ray pixel_ray = Ray(ray_base, view_direction);

    Intersection base_intersection = getBoxIntersection(pixel_ray, octreeBound);

    float base_factor = step_0(base_intersection.t_far - base_intersection.t_near);    
    if (base_factor == 0) {
        discard;
    }

    if (auto_lod == 1) {
        //float leaf_size = (upper.x - lower.x) / pow(2, max_tree_depth);
        int dist_factor = int(base_intersection.t_far / 25);
        max_depth -= dist_factor;
        max_depth = max(min_render_depth + 1, max_depth);
    }
    vec3 position = ray_base + base_intersection.t_near * view_direction + ray_eps;

    vec3 outline_color = (position - lower) / (higher - lower);

    vec3 sggx_sum_color = vec3(0);
    vec3 sggx_color = vec3(1);

    float roentgen_count = 0;

    int pixel_samples = 1;

    for (int i = 0; i < pixel_samples; i++) {
        float U1 = rand(gl_FragCoord.xy + i * world_pos.xy + 43 * min_render_depth * world_pos.yz + 47 * max_render_depth * world_pos.xz);
        float U2 = rand(gl_FragCoord.yz + U1 * world_pos.xy + 53 * min_render_depth * world_pos.yz + 57 * max_render_depth * world_pos.xz);

        float upFactor = U1 - 0.5;
        float tangentFactor = U2 - 0.5;

        vec3 up = vec3(0, 1, 0);
        vec3 tangent = cross(view_direction, up);

        vec3 ray_target = world_pos.xyz + up * upFactor * vertical_pixel_size + tangent * tangentFactor * horizontal_pixel_size;

        vec3 ray_base = camera_pos;
        vec3 ray_dir = normalize(ray_target - camera_pos);

        Ray pixel_ray = Ray(ray_base, ray_dir);

        Intersection outline_intersection = getBoxIntersection(pixel_ray, octreeBound);

        float factor = step_0(outline_intersection.t_far - outline_intersection.t_near);
        if (factor == 0) {
            continue;
        }

        // TODO do LOD calculations here
        

        float eps = 1e-4;
        vec3 ray_eps = ray_dir * eps;
        vec3 position = ray_base + outline_intersection.t_near * ray_dir + ray_eps;

        vec3 outline_color = (position - lower) / (higher - lower);

        uint parent_buf[max_stack_size];
        Stack parent_stack = Stack(parent_buf, 0);

        AABB aabb_stack_buf[max_stack_size];
        AABBStack aabb_stack = AABBStack(aabb_stack_buf, 0);

        AABB running_aabb = AABB(octreeBound.lower, octreeBound.upper);

        uint node_index = 0;
        uint current_depth = 0;

        // Find Leaf at Position
        bool result = traverseDownToLeaf(position, max_depth, current_depth, node_index, running_aabb, parent_stack, aabb_stack);
        Intersection leaf_intersection = getBoxIntersection(pixel_ray, running_aabb);

        float count = 0;
        float roentgen_count = 0;

        float remaining_contribution = 1.0;

        while (result && count < 200) {
            // Render Leaf
            count++;
            roentgen_count += step_0(float(current_depth) - min_render_depth);
            

            uint leaf_index = nodes_data[node_index].leaf_data_index;
            if (leaf_index < leaves_size) {
                leaf_node_s leaf = leaves_data[leaf_index];
                //vec3 normal = vec3(leaf.normal_x, leaf.normal_y, leaf.normal_z);

                if (output_type == 0 && abs(leaf.value) > 1e-3) {
                    vec3 color = evaluateSggx(leaf, ray_dir);
                    color *= 0.9;
                    color += 0.1 * vec3(1); 

                    //float length_base = length(running_aabb.upper - running_aabb.lower);
                    float length_base = abs(running_aabb.upper.x - running_aabb.lower.x);
                    float Intersection_length = leaf_intersection.t_far - leaf_intersection.t_near;

                    float length_factor = min(Intersection_length / length_base, 1);

                    // sggx_sum_color += remaining_contribution * length_factor * color;
                    sggx_sum_color += color;

                    // remaining_contribution *= (1 - length_factor);

                    //if (remaining_contribution < 0.01) break;

                    break;
                }
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
    }

    

    // Calculate output Color after traversal 
    if (output_type == 0) {
        sggx_sum_color /= pixel_samples;
        out_color = vec4(sggx_sum_color, 1);
    } else if (output_type == 1) {
        //out_color = vec4(float(count) / 50 * vec3(1, 0, 0.5), 1);
        vec3 color = vec3(1, 0, 0.5);
        out_color = vec4(roentgen_count / float(roentgen_denom) * color, 1);
    }

    // This renders an outline around the bounding box if AABBOutlineFactor uniform is set to 1
    out_color += AABBOutlineFactor * 0.3 * vec4(outline_color, 1);


    int horizontal_index = int(gl_FragCoord.x);
    int vertical_index = int(gl_FragCoord.y);

    int buffer_index = vertical_index * horizontal_pixels + horizontal_index;

    vec4 old_color = imageLoad(history_buffer, buffer_index);

    float alpha = 0.4;
    vec4 new_buffer_color = out_color * alpha + old_color * (1 - alpha);
    imageStore(history_buffer, buffer_index, new_buffer_color);

    out_color = new_buffer_color;

    imageStore(history_buffer, buffer_index, out_color);

    //out_color = vec4(1, 1, 0, 1);

    

    
}