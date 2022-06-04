#pragma once

#include <string>
#include <iostream>
#include <glm/glm.hpp>

#include "../GL_Utils.h"

typedef struct Shader_Definition_s {
	std::string vertex_path;
	std::string fragment_path;
}ShaderDefinition;

class Shader
{
private:
	unsigned int m_GlId;
	std::string vertPath = "";
	std::string fragPath = "";
	ShaderDefinition m_ShaderDef;

	bool valid = false;
public:
	Shader(std::string vertPath, std::string fragPath);

	Shader(ShaderDefinition m_ShaderDef);
	~Shader();

	void bind();
	void unbind();
	

	bool setUniform1i(const std::string& name, const unsigned int value);

	bool setUniform1f(const std::string& name, const float value);
	bool setUniform3f(const std::string& name, const glm::vec3& value);
	bool setUniform4f(const std::string &name, const glm::vec4 &value);
	bool setUniformMat4f(const std::string& name, const glm::mat4 &value);

	bool setUniform3fv(const std::string& name, const unsigned int count, const glm::vec3* data);

	void reloadShader();
	bool isValid() { return valid; }
private:
	unsigned int getUniformLocation(const std::string &name);
	void printUniformNotFound(std::string name);

	void createShader(const std::string& vertexShader, const std::string& fragmentShader);
	unsigned int compileShader(unsigned int type, const std::string& source);
};

