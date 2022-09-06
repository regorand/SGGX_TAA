#include "CameraPath.h"

#include <iostream>

#include "glm/gtx/compatibility.hpp"

CameraPath::CameraPath()
{
}

CameraPath::~CameraPath()
{
}

void CameraPath::addKeyframe(Keyframe keyframe, unsigned int frame)
{
    m_keyframes[frame] = keyframe;
    min_frame = glm::min(frame, min_frame);
    max_frame = glm::max(frame, max_frame);
}

Keyframe CameraPath::interpolateKeyframe(unsigned int frame)
{
    if (m_keyframes.size() <= 0) return { glm::vec3(0, 0, 0), glm::vec3(0, 0, 0) };
    if (m_keyframes.size() == 1) return m_keyframes.begin()->second;
    if (m_keyframes.size() == 2) {
        auto it = m_keyframes.begin();
        unsigned int frame1 = it->first;
        Keyframe kf1 = it->second;
        it++;
        unsigned int frame2 = it->first;
        Keyframe kf2 = it->second;
        float t = ((float) frame - frame1) / ((float) frame2 - frame1);
        glm::vec3 interpolated_pos = glm::lerp(kf1.position, kf2.position, t);

        // TODO: use quaternion slerp instead of interpolation euler angles
        glm::vec3 interpolated_rot = glm::lerp(kf1.rotation, kf2.rotation, t);

        return { interpolated_pos, interpolated_rot };
    }

    auto it = m_keyframes.begin();
    
    //make keyframes loop
    Keyframe kf1 = m_front_keyframe;
    Keyframe kf2 = it->second;
    unsigned int f1 = it->first;

    if (f1 > frame) {
        // frame number lower than first frame, clamp to first keyframe we have
        return kf1;
    }

    it++;
    Keyframe kf3 = it->second;
    unsigned int f2 = it->first;
    it++;
    Keyframe kf4 = it->second;

    while (f2 < frame) {
        f1 = f2;
        f2 = it->first;

        kf1 = kf2;
        kf2 = kf3;
        kf3 = kf4;

        // if we are at the last keyframe, use back keyframe
        it++;
        if (it == m_keyframes.end()) {
            kf4 = m_back_keyframe;
            break;
        }
        
        kf4 = it->second;
    }

    if (frame > f2) {
        //frame number higher than last frame, clamp to last frame
        return kf4;
    }

    float t = ((float)frame - f1) / ((float)f2 - f1);

    glm::vec3 interpolated_pos = CatmullRom(kf1.position, kf2.position, kf3.position, kf4.position, t);

    return { interpolated_pos, glm::vec3(0, 0, 0) };
}

unsigned int CameraPath::getMinFrame()
{
    return min_frame;
}

unsigned int CameraPath::getMaxFrame()
{
    return max_frame;
}

void CameraPath::setFrontKeyframe(Keyframe kf)
{
    m_front_keyframe = kf;
}

void CameraPath::setBackKeyframe(Keyframe kf)
{
    m_back_keyframe = kf;
}

void CameraPath::setLinearEndKeyframes()
{
    if (m_keyframes.size() < 2) return;

    auto it = m_keyframes.begin();
    auto kf1 = it->second;
    it++;
    auto kf2 = it->second;
    glm::vec3 front_pos = kf1.position + (kf1.position - kf2.position);
    m_front_keyframe = { front_pos, glm::vec3(0) };
    auto back_it = m_keyframes.rbegin();
    kf1 = back_it->second;
    back_it++;
    kf2 = back_it->second;
    glm::vec3 back_pos = kf1.position + (kf1.position - kf2.position);
    m_back_keyframe = { back_pos, glm::vec3(0) };
}


float CameraPath::GetT(float t, float alpha, const glm::vec3& p0, const glm::vec3& p1)
{
    auto d = p1 - p0;
    float a = glm::dot(d, d); // d | d; // Dot product
    float b = glm::pow(a, alpha * .5f);
    return (b + t);
}

// Interpolation Code inspired from Wikipedia 
// https://en.wikipedia.org/wiki/Centripetal_Catmull%E2%80%93Rom_spline (03.09.22)
glm::vec3 CameraPath::CatmullRom(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t, float alpha)
{
    float t0 = 0.0f;
    float t1 = GetT(t0, alpha, p0, p1);
    float t2 = GetT(t1, alpha, p1, p2);
    float t3 = GetT(t2, alpha, p2, p3);

    t = glm::lerp(t1, t2, t);

    glm::vec3 A1 = (t1 - t) / (t1 - t0) * p0 + (t - t0) / (t1 - t0) * p1;
    glm::vec3 A2 = (t2 - t) / (t2 - t1) * p1 + (t - t1) / (t2 - t1) * p2;
    glm::vec3 A3 = (t3 - t) / (t3 - t2) * p2 + (t - t2) / (t3 - t2) * p3;
    glm::vec3 B1 = (t2 - t) / (t2 - t0) * A1 + (t - t0) / (t2 - t0) * A2;
    glm::vec3 B2 = (t3 - t) / (t3 - t1) * A2 + (t - t1) / (t3 - t1) * A3;
    glm::vec3 C = (t2 - t) / (t2 - t1) * B1 + (t - t1) / (t2 - t1) * B2;
    return C;
}
