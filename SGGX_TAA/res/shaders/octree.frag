#version 430 core

struct octree_node_s {
	uint type_and_index; 
	uint parent_index;
    uint leaf_data_index;
};

struct inner_node_s {
	uint node_indices[8];
};

struct leaf_node_s {
	float value;
    float normal_x;
    float normal_y;
    float normal_z;
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

uniform int inner_nodes_offset;
uniform int leaves_offset;

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


uniform vec3 lower;
uniform vec3 higher;
//uniform float AABBOutlineFactor;

uniform vec3 camera_pos;


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

in vec4 world_pos;

out vec4 out_color;

void main() {
    out_color = vec4(0);

    AABB octreeBound = AABB(lower, higher);
    
    vec3 ray_base = camera_pos;
    vec3 ray_dir = normalize(world_pos.xyz - camera_pos);

    Ray pixel_ray = Ray(ray_base, ray_dir);
    Intersection outline_intersection = getBoxIntersection(pixel_ray, octreeBound);

    float factor = step_0(outline_intersection.t_far - outline_intersection.t_near);
    if (factor == 0) {
        discard;
    }

    // TODO do LOD calculations here
    int max_depth = max_render_depth;
    if (auto_lod == 1) {
        //float leaf_size = (upper.x - lower.x) / pow(2, max_tree_depth);

        int dist_factor = int(outline_intersection.t_far / 25);

        max_depth -= dist_factor;
        max_depth = max(min_render_depth + 1, max_depth);
    }

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

    // Any outs here mean we don't fully traverse the Octree
    if (output_type == 2) {
        bool result = traverseDownToLeaf(position, max_depth, current_depth, node_index, running_aabb, parent_stack, aabb_stack);

        if (!result) return;

        Intersection leaf_intersection = getBoxIntersection(pixel_ray, running_aabb);
        out_color = (leaf_intersection.t_far - leaf_intersection.t_near) / 2 * vec4(1);
        return;
    }

    // From here: Full Octree traversal

    // Find Leaf at Position
    bool result = traverseDownToLeaf(position, max_depth, current_depth, node_index, running_aabb, parent_stack, aabb_stack);
    Intersection leaf_intersection = getBoxIntersection(pixel_ray, running_aabb);

    float count = 0;
    float roentgen_count = 0;
    
    vec3 sggx_sum_color = vec3(0);
    vec3 sggx_color = vec3(1);
    float remaining_contribution = 1.0;


    while (result && count < 200) {
        // Render Leaf
        count++;
        roentgen_count += step_0(float(current_depth) - min_render_depth);

        uint leaf_index = nodes_data[node_index].leaf_data_index;
        if (leaf_index < leaves_size) {
            leaf_node_s leaf = leaves_data[leaf_index];
            vec3 normal = vec3(leaf.normal_x, leaf.normal_y, leaf.normal_z);

            float brightness_factor = 3;
            float density_base = leaf.value;
            float length_base = 2 * (running_aabb.upper.x - running_aabb.lower.x);
            float Intersection_length = leaf_intersection.t_far - leaf_intersection.t_near;
            float length_factor = min(Intersection_length / length_base, 1);

            float ambient_factor = 0.2;

            vec3 light_dir = vec3(1, 1, 0);
            float light_normal_dot = max(dot(light_dir, normal), 0);
            float diffuse_factor = (1 - ambient_factor) * light_normal_dot;

            float max_depth_factor = pow(2, max(max_tree_depth - max_depth, 0));

            float factor = brightness_factor * length_factor * density_base * max_depth_factor;


            sggx_sum_color += (ambient_factor + diffuse_factor) * remaining_contribution * factor * sggx_color;
            remaining_contribution *= (1 - factor);

            //float rand_val = rand(position.xy);
            if (output_type == 0 && remaining_contribution < 0.01) break;
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
        out_color = vec4(sggx_sum_color, 1);
    } else if (output_type == 1) {
        //out_color = vec4(float(count) / 50 * vec3(1, 0, 0.5), 1);
        vec3 color = vec3(1, 0, 0.5);
        out_color = vec4(roentgen_count / float(roentgen_denom) * color, 1);
    }

    // This renders an outline around the bounding box if AABBOutlineFactor uniform is set to 1
    out_color += AABBOutlineFactor * 0.3 * vec4(outline_color, 1);
}