#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>

#include "../Parameters.h"

class Camera
{
private:
	glm::vec3 position;
	glm::vec3 viewDirection;

	const glm::vec3 up = glm::vec3(0, 1, 0);

public:
	Camera();
	Camera(glm::vec3 position, glm::vec3 viewDirection);
	~Camera();

	glm::mat4 getViewMatrix();
	glm::vec3 getPosition();

	void setPosition(glm::vec3 newPosition);
	void setViewDiretion(glm::vec3 newViewDirection);
	void setLookAt(glm::vec3 target);

	std::vector<glm::vec3> getScreenCoveringQuad();

	void update();
};

