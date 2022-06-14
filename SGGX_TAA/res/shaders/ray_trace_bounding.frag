#version 430 core

in vec4 world_pos;

out vec4 out_color;

uniform vec3 camera_pos;
uniform float step_distance;
uniform int max_steps;
uniform float AABBOutlineFactor;

struct AABB {
    vec3 lower;
    vec3 upper;
};

const float minStepDistance = 0.005; // used to prevent division by zero error
const int binary_search_steps = 5;
const float h = 0.01;
const float ambientFactor = 0.2;
const float diffuseFactor = 1 - ambientFactor;
const vec3 up = vec3(0, 1, 0);


vec3 getAABBCenter(AABB aabb) {
    return (aabb.lower + aabb.upper) / 2;
}
// if a > 0, this returns 1, otherwise 0
float clamp_to_0_1(float a) {
    return max(sign(a), 0);
}

float intersect(float f1, float f2) {
    return max(f1, f2);
}

float combine(float f1, float f2) {
    return min(f1, f2);
}

float subtract(float f1, float f2) {
    return max(f1, -f2);
}

float boxSDF( vec3 p, vec3 b )
{
    vec3 q = abs(p) - b;
    return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float sphereSDF(vec3 center, float radius, vec3 point) {
    return length(center - point) - radius;
}

float sdf(vec3 current) {
    //return max(sphereSDF(vec3(0, 0, -3), 1, current), -sphereSDF(vec3(0, 1, -3), 1.8 , current));
    //return min(sphereSDF(vec3(0, 0, -3), 1, current), sphereSDF(vec3(1, 1, -3), 1, current));
    //return sphereSDF(vec3(0, 0, -2), 0.9, current);

    return intersect(boxSDF(current, vec3(0.5)), sphereSDF(vec3(0, 0, 0), 0.5, current));
    //return sphereSDF(vec3(0), 0.5, current);
}

vec3 findIntersection(float dist, vec3 current, vec3 diretion) {
    for (int i = 0; i < binary_search_steps; i++) {
        current += sign(dist) * diretion / pow(2, i);
        dist = sdf(current);
    }
    return current;
}

vec3 calculateNormal(vec3 position) {
    return normalize(vec3(  sdf(position + vec3(h, 0, 0)) - sdf(position - vec3(h, 0, 0)), 
                            sdf(position + vec3(0, h, 0)) - sdf(position - vec3(0, h, 0)), 
                            sdf(position + vec3(0, 0, h)) - sdf(position - vec3(0, 0, h))));
}

void main() {   
    AABB cube = AABB(vec3(-1), vec3(-2));

    vec3 sphere_color = vec3(1, 0, 0);

    vec3 ray_base = camera_pos;
    vec3 ray_dir = normalize(world_pos.xyz - camera_pos);

    float t_near = -1;
    float t_far = 1000;

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
    out_color = vec4(0);
    
    ray_base = ray_base + t_near * ray_dir;
    vec3 center = getAABBCenter(cube); 
    //out_color = factor * sqrt(2) * (length(intersection - center) - 0.5) * vec4(1, 1, 1, 1);

    float actualStepDist = max(step_distance, minStepDistance);
    float neededSteps = (t_far - t_near) / actualStepDist;
    vec3 ray_step = ray_dir * actualStepDist;
    vec3 transformed = ray_base - center;

    for (int i = 0; i < neededSteps; i++) {
        transformed += ray_step;
        float dist = sdf(transformed);
        if (dist <= 0) {
            vec3 intersection = findIntersection(dist, transformed, ray_step);
            vec3 normal = calculateNormal(intersection);

            out_color = vec4(ambientFactor * sphere_color, 1);
            out_color += vec4(diffuseFactor * max(dot(up, normal), 0) * sphere_color, 1);
            break;
        }
    }
    out_color += AABBOutlineFactor * factor * vec4(1) / 5;
}
