#include "Material.h"

Material::Material()
	:m_KA(glm::vec3(0)), m_KD(glm::vec3(0)), m_KS(glm::vec3(0)), m_SpecularCoeff(0.0f) {}

Material::Material(serializable_material source)
	:	m_KA(glm::make_vec3<float>(source.kA)), 
		m_KD(glm::make_vec3<float>(source.kD)),
		m_KS(glm::make_vec3<float>(source.kS)),
		m_SpecularCoeff(source.specularCoeff) {
	diffuse_texname = std::string(source.diffuse_texname);
}

Material::Material(glm::vec3 KA, glm::vec3 KD, glm::vec3 KS, float specularCoeff)
	:m_KA(KA), m_KD(KD), m_KS(KS), m_SpecularCoeff(specularCoeff) {}

Material::~Material() {}

void Material::serialize(serializable_material& target) 
{
	for (int i = 0; i < 3; i++) {
		target.kA[i] = m_KA[i];
		target.kD[i] = m_KD[i];
		target.kS[i] = m_KS[i];
	}
	auto len = glm::min(diffuse_texname.size(), (size_t)254);
	memcpy(target.diffuse_texname, diffuse_texname.c_str(), len + 1);
	target.specularCoeff = m_SpecularCoeff;
}
