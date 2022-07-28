#pragma once

#include <glm/glm.hpp>

#include "../geometry/Utils.h"
#include "RasterizationObject.h"
#include "SceneObject.h"
#include "Renderer.h"
#include "Camera.h"
#include "../gl_wrappers/Texture.h"
#include "Light.h"
#include "RayMarchObject.h"
#include "../geometry/VoxelGrid.h"
#include "../utils/Loader.h"

class SceneController
{
private:
	std::map<std::string, std::shared_ptr<SceneObject>> sceneObjects;
	std::shared_ptr<SceneObject> activeObject = nullptr;

	std::vector<std::shared_ptr<Light>> sceneLights;

	std::shared_ptr<RayMarchObject> rayObj;
	std::shared_ptr<VoxelGrid> m_voxels;

	std::string currentlyLoadingPath;

	Renderer renderer;
	Camera camera;
	Loader loader;

public:
	SceneController();
	~SceneController();

	
	bool init();
	void doFrame();

	void updateModels();
	void updateCamera();
	void lookAtObject();
	std::shared_ptr<RayMarchObject> setupRayMarchingQuad();

	void loadAndDisplayObject(std::string object_path);

	void reloadShaders();
	void reloadOctreeVis();

private:

	bool switchRenderedObject(std::string path);
	bool removeSceneObject(std::string path, bool evenIfActive = false);
};
