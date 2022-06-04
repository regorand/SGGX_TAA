#include "Material.h"

Material::Material(glm::vec3 KA, glm::vec3 KD, glm::vec3 KS, float specularCoeff):
	m_KA(KA), m_KD(KD), m_KS(KS), m_SpecularCoeff(specularCoeff) {}

Material::~Material() {}
