#include "Light.h"

Light::Light(glm::vec3 position, glm::vec3 color)
	: position(position), color(color)
{}

void Light::setPosition(const glm::vec3 new_position)
{
	position = new_position;
}

glm::vec3 Light::getPosition()
{
	return position;
}

glm::vec3 Light::getLightColor()
{
	return color;
}
