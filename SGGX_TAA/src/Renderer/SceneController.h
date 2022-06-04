#pragma once

#include <glm/glm.hpp>

#include "../Utils.h"
#include "SceneObject.h"
#include "Renderer.h"
#include "Camera.h"
#include "../gl_wrappers/Texture.h"
#include "Light.h"
#include "RayMarchObject.h"

class SceneController
{
private:
	std::vector<std::shared_ptr<SceneObject>> sceneObjects;
	std::vector<std::shared_ptr<Light>> sceneLights;

	std::shared_ptr<RayMarchObject> rayObj;

	Renderer renderer;
	Camera camera;

	float angleY = 0;
	float angle_speed_Y = 0.01f;

public:
	SceneController();
	~SceneController();

	
	bool init();
	void doFrame();

	void updateCamera();
	std::shared_ptr<RayMarchObject> setupRayMarchingQuad();

	void reloadShaders();
};

std::shared_ptr<SceneObject> registerSceneObject(Mesh_Object_t &source, std::shared_ptr<Shader> shader, glm::mat4 model_matrix);
std::shared_ptr<RayMarchObject> setupRayMarchingQuad();
