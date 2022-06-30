#version 430 core
struct voxel_data_object {
    float value;
    float x;
    float y;
    float z;
};

layout(std430, binding = 0) buffer voxels
{
    voxel_data_object data[];
};

uniform int dimension;
uniform float voxel_size;
uniform vec3 lower;
uniform vec3 higher;
uniform int output_type;

uniform vec3 camera_pos;

struct AABB {
    vec3 lower;
    vec3 upper;
};

vec3 getAABBCenter(AABB aabb) {
    return (aabb.lower + aabb.upper) / 2;
}

float clamp_to_0_1(float a) {
    return max(sign(a), 0);
}

vec3 getIndexFromPosition(vec3 position) {
    return floor((position - lower) / voxel_size);
}

bool continueLoop(vec3 voxelIndex, int dimension) {
    return voxelIndex.x < dimension && voxelIndex.x >= 0 
        && voxelIndex.y < dimension && voxelIndex.y >= 0
        && voxelIndex.z < dimension && voxelIndex.z >= 0;
}

in vec4 world_pos;
out vec4 out_color;

void main() {   
    out_color = vec4(0);
    AABB cube = AABB(lower, higher);

    vec3 ray_base = camera_pos;
    vec3 ray_dir = normalize(world_pos.xyz - camera_pos);

    float t_near = -1;
    float t_far = 100000;

    for (int i = 0; i < 3; i++) {
        float d = ray_dir[i];
        if (d != 0) {
            float t1 = (cube.lower[i] - ray_base[i]) / d;
            float t2 = (cube.upper[i] - ray_base[i]) / d;
            if (t1 > t2) {
                float tmp = t2;
                t2 = t1;
                t1 = tmp;
            }
            t_near = max(t_near, t1);
            t_far = min(t_far, t2);
        }
    }

    float factor = clamp_to_0_1(t_far - t_near);
    if (factor == 0) {
        discard;
    }
    ray_base = ray_base + t_near * ray_dir;
    
    float step_dist = voxel_size * 0.1;
    float eps_dist = voxel_size * 0.001;
    ray_base += ray_dir * eps_dist;

    //vec3 voxelIndex = floor(vec3(ray_base - lower) / voxel_size);
    vec3 voxelIndex = getIndexFromPosition(ray_base);
    
    /*
    voxelIndex.x = min(voxelIndex.x, dimension - 1);
    voxelIndex.y = min(voxelIndex.y, dimension - 1);
    voxelIndex.z = min(voxelIndex.z, dimension - 1);
    */
    vec3 up = normalize(vec3(0, 1, 0));

    
    while (continueLoop(voxelIndex, dimension)) {
        voxel_data_object obj = data[int(voxelIndex.x) + int(voxelIndex.y) * dimension + int(voxelIndex.z) * dimension * dimension];
        //out_color += (1 - diff_xy) * (1 - diff_xz) * vec4(1, 1, 1, 0);
        float val = obj.value;
        if (val == 1) {
            vec3 normal = vec3(obj.x, obj.y, obj.z);
            if (output_type == 0) {
                out_color = vec4(1);
            } else if (output_type == 1) {
                out_color = vec4(vec3(dot(normal, up)), 1);
            } else if (output_type == 2) {
                out_color = vec4(normal, 1);
            } else if (output_type == 3) {
                out_color = vec4(abs(normal), 1);
            } else if (output_type == 4) {
                out_color = vec4(world_pos.xyz / world_pos.w, 1);
            } else if (output_type == 5) {
                out_color = factor * vec4(voxelIndex / dimension, 1);
            }
            
            //
            break;
        }
        ray_base += ray_dir * step_dist;
        voxelIndex = getIndexFromPosition(ray_base);
    }
    


    vec3 frac = fract(vec3(1) - abs((ray_base - lower) / voxel_size));
    vec3 nexts = frac / abs(ray_dir);
    //nexts = sqrt(sqrt(nexts));
    float maxVal = max(max(nexts.x, nexts.y), nexts.z);
    float minVal = min(min(nexts.x, nexts.y), nexts.z);
    
    //out_color = factor * vec4(voxelIndex / dimension, 1);

    vec3 deltas = vec3(voxel_size) / ray_dir;
    vec3 signs = sign(deltas);
    vec3 abs_deltas = abs(deltas);

    float maxDelta = max(max(abs_deltas.x, abs_deltas.y), abs_deltas.z);
    //out_color = vec4(vec3(minVal), 1);

    /*
    out_color = vec4(0, 0, 0, 1);
    while (continueLoop(voxelIndex, dimension)) {
        voxel_data_object obj = data[int(voxelIndex.x) + int(voxelIndex.y) * dimension + int(voxelIndex.z) * dimension * dimension];
        out_color += vec4(vec3(obj.value), 1);
        int next = 3;
        if (nexts.x < nexts.y && nexts.x < nexts.z) {
            next = 0;
        }
        else if (nexts.y < nexts.z) {
            next = 1;
        }
        else {
            next = 2;
        }
        
        if (next != 3) {
            voxelIndex[next] += signs[next];
            nexts[next] += abs_deltas[next];
        }
    }
    */

    //out_color *= factor;
    //out_color = vec4(factor, factor, factor, 1);
    //out_color = factor * vec4(count, count, count, 1) / 40;
    //out_color = factor * vec4(voxelIndex / dimension, 1);

}
