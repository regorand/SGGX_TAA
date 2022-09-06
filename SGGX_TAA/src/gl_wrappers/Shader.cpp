#include "Shader.h"

#include "../utils/file_utils.h"
#include <glm/gtc/type_ptr.hpp>
#include "../Parameters.h"

#include <vector>

Shader::Shader(std::string vertPath, std::string fragPath)
{
	m_ShaderDef.vertex_path = vertPath;
	m_ShaderDef.fragment_path = fragPath;

	reloadShader();
}

Shader::Shader(ShaderDefinition shaderDef)
	: m_ShaderDef(shaderDef)
{
	reloadShader();
}

Shader::~Shader()
{
	glDeleteProgram(m_GlId);
}

unsigned int Shader::getUniformLocation(const std::string name)
{
	if (name == "camera_pos") {
		int x = 0;
	}
	auto name_c_str = name.c_str();
	unsigned int res = glGetUniformLocation(m_GlId, name_c_str);
	return res;
}

void Shader::printUniformNotFound(std::string name)
{
	if (parameters.printUniformNotFound) {
		std::cout << "Could not find Uniform with name \"" << name << "\"" << std::endl;
	}
}

void Shader::reloadShader()
{
	std::string vertSrc = read_file(m_ShaderDef.vertex_path);
	std::string fragSrc = read_file(m_ShaderDef.fragment_path);
	createShader(vertSrc, fragSrc);
	if (valid) {
		glUseProgram(m_GlId);
	}
	unbind();
}

void Shader::createShader(const std::string& vertexShader, const std::string& fragmentShader)
{
	m_GlId = glCreateProgram();
	unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

	if (vs == 0 || fs == 0) {
		valid = false;
		return;
	}

	glAttachShader(m_GlId, vs);
	glAttachShader(m_GlId, fs);

	glLinkProgram(m_GlId);

	GLint isLinked = 0;
	glGetProgramiv(m_GlId, GL_LINK_STATUS, &isLinked);
	if (!isLinked) {
		GLint maxLength = 0;
		glGetProgramiv(m_GlId, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(m_GlId, maxLength, &maxLength, &infoLog[0]);

		std::string s(infoLog.begin(), infoLog.end());
		std::cout << "Info Log: " << s << std::endl;

		// The program is useless now. So delete it.
		glDeleteProgram(m_GlId);

		valid = false;
	}
	else {
		glValidateProgram(m_GlId);
		valid = true;
	}

	glDeleteShader(vs);

	glDeleteShader(fs);
}

void Shader::bind()
{
	glUseProgram(m_GlId);
}

void Shader::unbind()
{
	glUseProgram(0);
}

bool Shader::setUniform1i(const std::string& name, const unsigned int value)
{
	GLint isLinked = 0;
	glGetProgramiv(m_GlId, GL_LINK_STATUS, &isLinked);
	int uniformId = getUniformLocation(name);
	if (uniformId == -1) {
		printUniformNotFound(name);
		return false;
	}
	glUniform1i(uniformId, value);
	return true;
}

bool Shader::setUniform1f(const std::string& name, const float value)
{
	int uniformId = getUniformLocation(name);
	if (uniformId == -1) {
		printUniformNotFound(name);
		return false;
	}
	glUniform1f(uniformId, value);
	return true;
}

bool Shader::setUniform2f(const std::string& name, const glm::vec2& value)
{
	int uniformId = getUniformLocation(name);
	if (uniformId == -1) {
		printUniformNotFound(name);
		return false;
	}
	glUniform2f(uniformId, value.x, value.y);
	return true;
}

bool Shader::setUniform3f(const std::string& name, const glm::vec3& value)
{
	int uniformId = getUniformLocation(name);
	if (uniformId == -1) {
		printUniformNotFound(name);
		return false;
	}
	glUniform3f(uniformId, value.x, value.y, value.z);
	return true;
}

bool Shader::setUniform4f(const std::string& name, const glm::vec4 &value)
{
	int uniformId = getUniformLocation(name);
	if (uniformId == -1) {
		printUniformNotFound(name);
		return false;
	}
	glUniform4f(uniformId, value.x, value.y, value.z, value.w);
	return true;
}

bool Shader::setUniformMat4f(const std::string& name, const glm::mat4 &value)
{
	int uniformId = getUniformLocation(name);
	if (uniformId == -1) {
		printUniformNotFound(name);
		return false;
	}
	glUniformMatrix4fv(uniformId, 1, GL_FALSE, glm::value_ptr(value));
	return true;
}

bool Shader::setUniform3fv(const std::string& name, const unsigned int count, const glm::vec3* data)
{
	int uniformId = getUniformLocation(name);
	if (uniformId == -1) {
		printUniformNotFound(name);
		return false;
	}
	std::vector<float> raw_data;
	for (unsigned int i = 0; i < count; i++) {
		raw_data.push_back(data[i].x);
		raw_data.push_back(data[i].y);
		raw_data.push_back(data[i].z);
	}
	glUniform3fv(uniformId, count * 3, raw_data.data());
	return true;
}

unsigned int Shader::compileShader(unsigned int type, const std::string& source)
{
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	int res;
	glGetShaderiv(id, GL_COMPILE_STATUS, &res);
	if (res == GL_FALSE) {
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

		char* message = (char*) malloc(length * sizeof(char));

		glGetShaderInfoLog(id, length, &length, message);

		std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << std::endl;
		std::cout << message << std::endl;

		glDeleteShader(id);
		
		free(message);

		return 0;
	}

	return id;
}
