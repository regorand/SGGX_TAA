#pragma once

#include <glm/glm.hpp>
#include <string>

class Material
{
private:



public:
	Material(glm::vec3 KA, glm::vec3 KD, glm::vec3 KS, float specularCoeff);
	~Material();

	glm::vec3 m_KA;
	glm::vec3 m_KD;
	glm::vec3 m_KS;

	std::string diffuse_texname = "";

	float m_SpecularCoeff;
};