#include "Camera.h"

Camera::Camera()
{
	update();
}

Camera::Camera(glm::vec3 position, glm::vec3 viewDirection)
	:position(position), viewDirection(viewDirection)
{}

Camera::~Camera() {}

glm::mat4 Camera::getViewMatrix()
{
	return glm::lookAt(position, position + viewDirection, up);
}

glm::vec3 Camera::getPosition()
{
	return position;
}

void Camera::setPosition(glm::vec3 newPosition)
{
	position = newPosition;
}

void Camera::setViewDiretion(glm::vec3 newViewDirection)
{
	viewDirection = newViewDirection;
}

void Camera::setLookAt(glm::vec3 target)
{
	viewDirection = glm::normalize(target - position);
}

std::vector<glm::vec3> Camera::getScreenCoveringQuad()
{
	std::vector<glm::vec3> vertices;

	float y_dist = glm::tan(camera_params.fov / 2);
	float x_dist = y_dist * ((float) parameters.windowWidth) / parameters.windowHeight;

	viewDirection = glm::normalize(viewDirection);
	glm::vec3 h = position + viewDirection;
	glm::vec3 tangent = glm::cross(viewDirection, up);

	vertices.push_back(h - y_dist * up - x_dist * tangent);
	vertices.push_back(h - y_dist * up + x_dist * tangent);
	vertices.push_back(h + y_dist * up - x_dist * tangent);
	vertices.push_back(h + y_dist * up + x_dist * tangent);

	return vertices;
}

void Camera::update()
{
	position = glm::vec3(0, 0, -(camera_params.camera_dist * camera_params.camera_dist));
	viewDirection = glm::vec3(0, 0, -1);
}
