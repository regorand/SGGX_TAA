#include "Camera.h"

Camera::Camera()
{
	update();
}

Camera::Camera(glm::vec3 position, glm::vec3 viewDirection)
	:position(position), viewDirection(viewDirection), up(glm::vec3(0, 1, 0))
{}

Camera::~Camera() {}

glm::mat4 Camera::getViewMatrix()
{
	viewDirection = glm::normalize(viewDirection);
	return glm::lookAt(position, position + viewDirection, up);
}

void Camera::saveViewMat()
{
	m_prev_view_mat = getViewMatrix();
}

glm::mat4 Camera::getPrevViewMatrix() {
	return m_prev_view_mat;
}

glm::vec3 Camera::getPosition()
{
	return position;
}

glm::vec3 Camera::getUpVector()
{
	return up;
}

void Camera::setPosition(glm::vec3 newPosition)
{
	position = newPosition;
}

glm::vec3 Camera::getViewDirection()
{
	return viewDirection;
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

void Camera::setUpVector(glm::vec3 upVector)
{
	up = upVector;
}

void Camera::update()
{
	position = glm::vec3(0, 0, -(camera_params.camera_dist * camera_params.camera_dist));
	viewDirection = glm::vec3(0, 0, -1);
}

void Camera::getScreenCoveringQuadData(std::vector<glm::vec3>& vertices, float* width, float* height)
{
	float y_dist = glm::tan(camera_params.fov / 2);
	float x_dist = y_dist * ((float)parameters.windowWidth) / parameters.windowHeight;

	viewDirection = glm::normalize(viewDirection);
	glm::vec3 h = position + viewDirection;
	glm::vec3 tangent = glm::cross(viewDirection, up);

	vertices.push_back(h - y_dist * up - x_dist * tangent);
	vertices.push_back(h - y_dist * up + x_dist * tangent);
	vertices.push_back(h + y_dist * up - x_dist * tangent);
	vertices.push_back(h + y_dist * up + x_dist * tangent);

	*width = 2 * x_dist;
	*height = 2 * y_dist;
}
