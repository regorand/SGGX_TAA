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

void get_parent_AABB(inout AABB box, uint inner_index) {
    vec3 factors = get_bound_factors(inner_index);
    vec3 one_minus_factors = 1 - factors;
    vec3 new_lower = one_minus_factors * box.lower + factors * (2 * box.lower - box.upper);
    vec3 new_upper = factors * box.upper + one_minus_factors * (2 * box.upper - box.lower);
    box.lower = new_lower;
    box.upper = new_upper;
}

struct tree_traversal_leaf_s {
    uint node_index;
    uint depth;
    uint inner_index;
    vec3 lower;
    vec3 higher;
};

tree_traversal_leaf_s getLeafAtPositionRayMarch(vec3 position, uint max_depth) {
    
    vec3 current_lower = lower;
    vec3 current_higher = higher;
    vec3 halfway = (current_higher + current_lower) / 2;

    uint node_index;
    octree_node_s current_node = nodes_data[0];
    uint index = NODE_INDEX(current_node.type_and_index);
    uint type = NODE_TYPE(current_node.type_and_index);

    uint depth = 0;
    uint inner_index = 0;

    while (type == INNER_NODE_TYPE && depth < max_depth) {
        depth++;
        inner_index = get_inner_index(AABB(current_lower, current_higher), position);

        vec3 factors = get_bound_factors(inner_index);
	    vec3 one_minus_factors = 1 - factors;
        current_higher = factors * current_higher + one_minus_factors * halfway;
	    current_lower = one_minus_factors * current_lower + factors * halfway;
        halfway = (current_higher + current_lower) / 2;

        node_index = inner_nodes_data[index].node_indices[inner_index];

        current_node = nodes_data[node_index];

        index = NODE_INDEX(current_node.type_and_index);
        type = NODE_TYPE(current_node.type_and_index);
    }
    return tree_traversal_leaf_s(node_index, depth, inner_index, current_lower, current_higher);
}

tree_traversal_leaf_s getLeafAtPosition(vec3 position, 
        uint max_depth, 
        inout uint current_depth,
        uint from_index, 
        AABB from_bound,
        inout Stack parent_stack, 
        //inout Stack inner_index_stack) 
        inout AABBStack aabb_stack) 
{    
    vec3 current_lower = from_bound.lower;
    vec3 current_higher = from_bound.upper;
    vec3 halfway = (current_higher + current_lower) / 2;

    uint node_index = from_index;
    octree_node_s current_node = nodes_data[node_index];
    uint index = NODE_INDEX(current_node.type_and_index);
    uint type = NODE_TYPE(current_node.type_and_index);

    uint inner_index = 0;

    while (type == INNER_NODE_TYPE && current_depth < max_depth) {
        current_depth++;
        inner_index = get_inner_index(AABB(current_lower, current_higher), position);

        if (hasCapacity(parent_stack)) {
            push(parent_stack, node_index);
        }
        /*
        if (hasCapacity(inner_index_stack)) {
            push(inner_index_stack, inner_index);
        }
        */
        if (hasCapacity(aabb_stack)) {
            push(aabb_stack, AABB(current_lower, current_higher));
        }

        //vec3 factors = vec3((inner_index & 0x4) >> 2, (inner_index & 0x2) >> 1, inner_index & 0x1);
        vec3 factors = get_bound_factors(inner_index);
	    vec3 one_minus_factors = 1 - factors;
        current_higher = factors * current_higher + one_minus_factors * halfway;
	    current_lower = one_minus_factors * current_lower + factors * halfway;
        halfway = (current_higher + current_lower) / 2;

        node_index = inner_nodes_data[index].node_indices[inner_index];

        current_node = nodes_data[node_index];

        index = NODE_INDEX(current_node.type_and_index);
        type = NODE_TYPE(current_node.type_and_index);
    }
    return tree_traversal_leaf_s(node_index, current_depth, inner_index, current_lower, current_higher);
}

bool getLeafAtPositionSafe(vec3 position, 
        uint max_depth, 
        inout uint current_depth,
        inout uint node_index, 
        inout AABB bounding_box,
        inout Stack parent_stack, 
        inout AABBStack aabb_stack) 
{    
    vec3 halfway = (bounding_box.upper + bounding_box.lower) / 2;

    octree_node_s current_node = nodes_data[node_index];
    uint index = NODE_INDEX(current_node.type_and_index);
    uint type = NODE_TYPE(current_node.type_and_index);

    uint inner_index = 0;

    while (type == INNER_NODE_TYPE && current_depth < max_depth) {
        current_depth++;
        inner_index = get_inner_index(bounding_box, position);

        if (!hasCapacity(parent_stack)) {
            return false;
        }
        push(parent_stack, node_index);

        if (!hasCapacity(aabb_stack)) {
            return false;
        }
        push(aabb_stack, bounding_box);

        //vec3 factors = vec3((inner_index & 0x4) >> 2, (inner_index & 0x2) >> 1, inner_index & 0x1);
        vec3 factors = get_bound_factors(inner_index);
	    vec3 one_minus_factors = 1 - factors;
        bounding_box.upper = factors * bounding_box.upper + one_minus_factors * halfway;
	    bounding_box.lower = one_minus_factors * bounding_box.lower + factors * halfway;
        halfway = (bounding_box.upper + bounding_box.lower) / 2;

        if (index >= inner_nodes_size) return false;
        node_index = inner_nodes_data[index].node_indices[inner_index];

        if (node_index >= nodes_size) return false;
        current_node = nodes_data[node_index];

        index = NODE_INDEX(current_node.type_and_index);
        type = NODE_TYPE(current_node.type_and_index);
    }
    return true;
}


bool backUpToNodeContaining(vec3 position,
        inout uint node_index,
        inout AABB node_AABB,
        inout Stack parent_stack,
        //inout Stack inner_index_stack)
        inout AABBStack aabb_stack)
{
    while (!isInBox(node_AABB, position)) {
        if (!hasElement(parent_stack) || !hasElement(aabb_stack)) {
            return false;
        }

        node_index = pop(parent_stack);
        node_AABB = pop(aabb_stack);

        /*
        uint inner = pop(inner_index_stack);

        vec3 factors = get_bound_factors(inner);
        vec3 one_minus_factors = 1 - factors;
        vec3 new_lower = one_minus_factors * node_AABB.lower + factors * (2 * node_AABB.lower - node_AABB.upper);
        vec3 new_upper = factors * node_AABB.upper + one_minus_factors * (2 * node_AABB.upper - node_AABB.lower);
        node_AABB.lower = new_lower;
        node_AABB.upper = new_upper;
        */

        //get_parent_AABB(node_AABB, inner);
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
    
    vec3 outline_color = vec3(0);

    if (output_type == 0) {
        
        uint max_depth = 5;

        float eps = 1e-5;
        vec3 ray_eps = ray_dir * eps;

        vec3 position = ray_base + outline_intersection.t_near * ray_dir + ray_eps;

        uint max_steps = 2000;
        float step_size = 0.005;
        vec3 ray_step = ray_dir * step_size;

        tree_traversal_leaf_s leaf_node  = getLeafAtPositionRayMarch(position, max_depth);

        octree_node_s node = nodes_data[leaf_node.node_index];
        AABB leaf_AABB = AABB(leaf_node.lower, leaf_node.higher);
        Intersection leaf_intersection = getBoxIntersection(pixel_ray, leaf_AABB);

        uint last_node_index = 0;
        for (int i = 0; i < max_steps; i++) {
            position += ray_step;
            if (!isInBox(octreeBound, position)) break;
            tree_traversal_leaf_s leaf_node  = getLeafAtPositionRayMarch(position, max_depth);
            octree_node_s node = nodes_data[leaf_node.node_index];

            leaf_node_s leaf = leaves_data[node.leaf_data_index];
            if (last_node_index != leaf_node.node_index) {
                out_color += (leaf.value) * vec4(1);
                last_node_index = leaf_node.node_index;
            }
        }
    } else if (output_type == 1) {
        float eps = 1e-4;
        vec3 ray_eps = ray_dir * eps;

        vec3 position = ray_base + outline_intersection.t_near * ray_dir + ray_eps;
        outline_color = (position - lower) / (higher - lower);

        // TODO, make max depth Uniform
        uint max_depth = 8;
        uint current_depth = 0;

        uint parent_buf[32];
        Stack parent_stack = Stack(parent_buf, 0);

        AABB aabb_stack_buf[32];
        AABBStack aabb_stack = AABBStack(aabb_stack_buf, 0);

        AABB runningBound = AABB(octreeBound.lower, octreeBound.upper);

        uint node_index = 0;

        // Step 1: First iteration outside loop ?
        bool result = getLeafAtPositionSafe(position, 
                max_depth, 
                current_depth, 
                node_index,
                runningBound, 
                parent_stack, 
                aabb_stack);

        if (!result) {
            return;
        }

        if (node_index >= nodes_size) return;
        octree_node_s node = nodes_data[node_index];
        Intersection leaf_intersection = getBoxIntersection(pixel_ray, runningBound);

        float count = 0;
        while (isInBox(octreeBound, position)) {
            count++;
            // Step 2: Render Color -> Eventually: Do SGGX here
            if (node.leaf_data_index >= leaves_size) break;
            leaf_node_s leaf = leaves_data[node.leaf_data_index];

            if (leaf.value != 0) {
                //out_color += (leaf.value / 10) * vec4(1);
                out_color = vec4(1);
            }

            // Step 3: get exit index and next node direction
            position = ray_base + leaf_intersection.t_far * ray_dir + ray_eps;

            result = backUpToNodeContaining(position, node_index, runningBound, parent_stack, aabb_stack);

            if (!result) {
                break;
            }

            //tree_traversal_leaf_s leaf_node  = getLeafAtPositionRayMarch(position, max_depth);

            /*
            tree_traversal_leaf_s leaf_node = getLeafAtPositionSafe(position, 
                    max_depth, 
                    current_depth, 
                    node_index,
                    leaf_AABB, 
                    parent_stack, 
                    aabb_stack);
            */
            
            result = getLeafAtPositionSafe(position, 
                        max_depth, 
                        current_depth, 
                        node_index,
                        runningBound, 
                        parent_stack, 
                        aabb_stack);

            if (node_index >= nodes_size) break;
            node = nodes_data[node_index];
            leaf_intersection = getBoxIntersection(pixel_ray, runningBound);

            // Idea: 
            // 3.1 calculate new Position
            // 3.2 move up to parent node (if going to root from root: Done (will be empty stack))
            // 3.3 do AABB check
            // 3.4 (a) If not in AABB, go to 3.2
            // 3.4 (b) Else get leaf

            // requires parent node Stack
            // requires either AABB Stack or inner_index stack
        }
    } else if (output_type == 2) {
        float eps = 1e-5;
        vec3 ray_eps = ray_dir * eps;

        vec3 position = ray_base + outline_intersection.t_near * ray_dir + ray_eps;
        vec3 outline_color = (position - lower) / (higher - lower);


        // TODO, make max depth Uniform
        uint max_depth = 5;
        uint current_depth = 0;


        uint parent_buf[32];
        Stack parent_stack = Stack(parent_buf, 0);
        AABB aabb_stack_buf[32];
        AABBStack aabb_stack = AABBStack(aabb_stack_buf, 0);

        uint inner_index_buf[32];    
        Stack inner_index_stack = Stack(inner_index_buf, 0);

        // Step 1: First iteration outside loop ?
        tree_traversal_leaf_s leaf_node  = getLeafAtPosition(position, 
                max_depth, 
                current_depth, 
                0,
                octreeBound, 
                parent_stack, 
                aabb_stack);

        octree_node_s node = nodes_data[leaf_node.node_index];
        AABB leaf_AABB = AABB(leaf_node.lower, leaf_node.higher);
        Intersection leaf_intersection = getBoxIntersection(pixel_ray, leaf_AABB);
        out_color = (leaf_intersection.t_far - leaf_intersection.t_near) / 2 * vec4(1);
    } else if (output_type == 3) {
        float eps = 1e-4;
        vec3 ray_eps = ray_dir * eps;

        vec3 position = ray_base + outline_intersection.t_near * ray_dir + ray_eps;
        int max_depth = 8;

        // Find Leaf at Position
        bool result = true;

        uint parent_buf[max_stack_size];
        Stack parent_stack = Stack(parent_buf, 0);

        uint inner_index_stack_buf[max_stack_size];
        Stack inner_index_stack = Stack(inner_index_stack_buf, 0);

        AABB aabb_stack_buf[max_stack_size];
        AABBStack aabb_stack = AABBStack(aabb_stack_buf, 0);

        AABB running_aabb = AABB(octreeBound.lower, octreeBound.upper);
        
        int current_depth = 0;
        uint node_index = 0;

        octree_node_s current_node = nodes_data[node_index];
        uint index = NODE_INDEX(current_node.type_and_index);
        uint type = NODE_TYPE(current_node.type_and_index);
        
        while (type == INNER_NODE_TYPE && current_depth < max_depth) {
            uint inner_index = get_inner_index(running_aabb, position);

            if (!hasCapacity(parent_stack)) {
                result = false;
                break;
            }
            if (!hasCapacity(inner_index_stack)) {
                result = false;
                break;
            }
            if (!hasCapacity(aabb_stack)) {
                result = false;
                break;
            }
            current_depth++;
            push(parent_stack, node_index);
            //push(inner_index_stack, inner_index);
            push(aabb_stack, AABB(running_aabb.lower, running_aabb.upper));
            //aabb_stack_buf[0] = AABB(running_aabb.lower, running_aabb.upper);

            downScaleAABB(running_aabb, inner_index);

            if (index >= inner_nodes_size) {
                result = false;
                break;
            }

            inner_node_s inner_node = inner_nodes_data[index];
            node_index = inner_node.node_indices[inner_index];

            if (node_index >= nodes_size) {
                result = false;
                break;
            }
            octree_node_s current_node = nodes_data[node_index];
            index = NODE_INDEX(current_node.type_and_index);
            type = NODE_TYPE(current_node.type_and_index);
        }

        Intersection leaf_intersection = getBoxIntersection(pixel_ray, running_aabb);

        float count = 0;
        while (result && count < 200) {
            // Render Leaf
            count++;

            position = ray_base + leaf_intersection.t_far * ray_dir + ray_eps;

            // Get new parent
            while (!isInBox(running_aabb, position)) {
                if (!hasElement(parent_stack) || !hasElement(aabb_stack)) {
                    result = false;
                    //return;
                    break;
                }

                node_index = pop(parent_stack);
                
                /*
                uint inner_index = pop(inner_index_stack);

                vec3 f = get_bound_factors(inner_index);
                out_color = vec4(f, 1);
                return;
                upScaleAABB(running_aabb, inner_index);
                */
                //push(aabb_stack, octreeBound);
                AABB tmp = pop(aabb_stack);
                //AABB tmp2 = pop(aabb_stack);
                //running_aabb = pop(aabb_stack);

                out_color = vec4(abs(tmp.upper) / 3, 1);
                //return; 

                running_aabb = tmp;
                current_depth--;
            }
            //break;
            if (!result) {
                //out_color = vec4(0, 0, 1, 1);
                //return;
                break;
            }
            
            
            // get Next Leaf
            if (node_index >= nodes_size) { 
                result = false;
                break;
            }
            octree_node_s current_node = nodes_data[node_index];
            index = NODE_INDEX(current_node.type_and_index);
            type = NODE_TYPE(current_node.type_and_index);

            if (type == INNER_NODE_TYPE) {
                out_color = vec4(1, 0, 0, 1);
                out_color = vec4(vec3(float(current_depth) / max_depth), 1);
                //return;
            }
            
            while (type == INNER_NODE_TYPE && current_depth < max_depth) {

                uint inner_index = get_inner_index(running_aabb, position);

                if (!hasCapacity(parent_stack)) {
                    result = false;
                    break;
                }

                if (!hasCapacity(aabb_stack)) {
                    result = false;
                    break;
                }
                current_depth++;
                push(parent_stack, node_index);
                push(aabb_stack, running_aabb);

                downScaleAABB(running_aabb, inner_index);

                if (index >= inner_nodes_size) {
                    result = false;
                    break;
                }

                inner_node_s inner_node = inner_nodes_data[index];
                node_index = inner_node.node_indices[inner_index];

                if (node_index >= nodes_size) {
                    result = false;
                    break;
                }
                octree_node_s current_node = nodes_data[node_index];
                index = NODE_INDEX(current_node.type_and_index);
                type = NODE_TYPE(current_node.type_and_index);
            }

            // Find leaf Intersection
            leaf_intersection = getBoxIntersection(pixel_ray, running_aabb);
            
            //result = false;
        }

        out_color = vec4(float(count) / 50 * vec3(1, 0, 0.5), 1);
        //out_color = vec4(vec3(float(parent_stack.current_size + 1) / 8), 1);
    } else if (output_type == 4) {
        vec3 c = vec3(0);
        c[outline_intersection.near_dimension] = 1;
        c[outline_intersection.far_dimension] = 1;
        out_color = vec4(c, 1);
    } 
    out_color += AABBOutlineFactor * 0.3 * vec4(outline_color, 1);
}