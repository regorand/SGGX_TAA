#pragma once

#include <map>
#include "glm/glm.hpp"

typedef struct keyframe_s {
	glm::vec3 position;
	glm::vec3 rotation;
} Keyframe;

class CameraPath
{
private:
	std::map<unsigned int, Keyframe> m_keyframes;

	// Catmull Intepolation uses 4 points to interpolate, these two are the ones to be used when interpolating the first or last 
	// of the actual camera positions
	Keyframe m_front_keyframe;
	Keyframe m_back_keyframe;

	unsigned int min_frame = 0xFFFFFFFF;
	unsigned int max_frame = 0;


public:
	CameraPath();
	~CameraPath();

	void addKeyframe(Keyframe keyframe, unsigned int frame);

	Keyframe interpolateKeyframe(unsigned int frame);

	unsigned int getMinFrame();
	unsigned int getMaxFrame();

	void setFrontKeyframe(Keyframe kf);
	void setBackKeyframe(Keyframe kf);
	void setLinearEndKeyframes();
private:
	float GetT(float t, float alpha, const glm::vec3& p0, const glm::vec3& p1);

	glm::vec3 CatmullRom(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t, float alpha = .5f);
};

