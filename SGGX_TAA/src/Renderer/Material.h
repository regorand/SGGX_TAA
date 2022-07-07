#pragma once

#include <glm/glm.hpp>
#include <string>
#include "glm/gtc/type_ptr.hpp"

typedef struct {
	float kA[3];
	float kD[3];
	float kS[3];
	char diffuse_texname[256];
	float specularCoeff;
} serializable_material;

class Material
{
private:

public:
	Material();
	Material(serializable_material source);
	Material(glm::vec3 KA, glm::vec3 KD, glm::vec3 KS, float specularCoeff);
	~Material();

	glm::vec3 m_KA;
	glm::vec3 m_KD;
	glm::vec3 m_KS;

	std::string diffuse_texname = "";

	float m_SpecularCoeff;

	void serialize(serializable_material& target);
};