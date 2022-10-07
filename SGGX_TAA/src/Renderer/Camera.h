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

	glm::vec3 up;

	glm::mat4 m_prev_view_mat;

public:
	Camera();
	Camera(glm::vec3 position, glm::vec3 viewDirection);
	~Camera();

	glm::mat4 getViewMatrix();
	glm::vec3 getPosition();
	glm::vec3 getUpVector();
	glm::vec3 getViewDirection();

	void setPosition(glm::vec3 newPosition);
	void setViewDiretion(glm::vec3 newViewDirection);
	void setLookAt(glm::vec3 target);
	void setUpVector(glm::vec3 upVector);

	std::vector<glm::vec3> getScreenCoveringQuad();
	void getScreenCoveringQuadData(std::vector<glm::vec3>& vertices, float* width, float* height);

	void update();

	void saveViewMat();
	glm::mat4 getPrevViewMatrix();
};

