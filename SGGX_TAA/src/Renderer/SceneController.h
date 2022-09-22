#pragma once

#include <glm/glm.hpp>

#include "../geometry/Utils.h"
#include "RasterizationObject.h"
#include "SceneObject.h"
#include "Renderer.h"
#include "Camera.h"
#include "CameraPath.h"
#include "../gl_wrappers/Texture.h"
#include "Light.h"
#include "RayMarchObject.h"
#include "../geometry/VoxelGrid.h"
#include "../utils/Loader.h"
#include "../Types.h"
#include "../utils/VideoExporter.h"


class SceneController
{
private:
	std::map<std::string, std::shared_ptr<SceneObject>> sceneObjects;
	std::shared_ptr<SceneObject> activeObject = nullptr;

	std::vector<std::shared_ptr<Light>> sceneLights;

	std::shared_ptr<RayMarchObject> rayObj;
	std::shared_ptr<VoxelGrid> m_voxels;

	std::shared_ptr<VideoExporter> m_videoExporter;

	std::string currentlyLoadingPath;

	Renderer renderer;

	Camera camera;
	CameraPath m_cameraPath;
	unsigned int m_cameraPathFrame = 0;
	bool m_cameraPathActive = false;

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

	void exportFrame();

	void loadAndDisplayObject(std::string object_path);

	void reloadShaders();
	void reloadOctreeVis();

	void updateCameraPathState(bool state);

private:

	bool updateRayMarchQuad();

	bool switchRenderedObject(std::string path);
	bool removeSceneObject(std::string path, bool evenIfActive = false);
};
