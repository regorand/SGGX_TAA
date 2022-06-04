#pragma once

#include <glm/glm.hpp>

class Light
{
private:
	glm::vec3 position;
	glm::vec3 color;

public:
	Light(glm::vec3 position, glm::vec3 color);

	void setPosition(const glm::vec3 new_position);

	glm::vec3 getPosition();
	glm::vec3 getLightColor();
};

