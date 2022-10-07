#include "SceneController.h"
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "../Parameters.h"

#include "../utils/file_utils.h"
#include "../geometry/Octree.h"

SceneController::SceneController()
	: camera(Camera(glm::vec3(0, 0, 1), glm::vec3(0, 0, -1))), m_voxels(nullptr), m_Exporter(std::make_shared<Exporter>())
{}

SceneController::~SceneController()
{}


bool SceneController::init()
{
	CameraPath path;

	m_cameraPath.addKeyframe({ glm::vec3(10, 0, -10), glm::vec3(-1, 0, 0) }, 0);
	m_cameraPath.addKeyframe({ glm::vec3(10, 0, 10), glm::vec3(-1, 0, 0) }, 180);

	/*
	m_cameraPath.addKeyframe({ glm::vec3(-10, 0, 0), glm::vec3(0, 0, 0) }, 360);
	m_cameraPath.addKeyframe({ glm::vec3(0, 0, -10), glm::vec3(0, 0, 0) }, 540);
	m_cameraPath.addKeyframe({ glm::vec3(10, 0, 0), glm::vec3(0, 0, 0) }, 720);
	*/

	m_cameraPath.setFrontKeyframe({ glm::vec3(10, 0, -20), glm::vec3(-1, 0, 0) });
	m_cameraPath.setBackKeyframe({ glm::vec3(10, 0, 10), glm::vec3(-1, 0, 0) });

	camera_params.min_frame = m_cameraPath.getMinFrame();
	camera_params.max_frame = m_cameraPath.getMaxFrame();

	/*
	sceneLights.push_back(std::make_shared<Light>(glm::vec3(10, 0, 0), glm::vec3(1, 1, 1)));
	sceneLights.push_back(std::make_shared<Light>(glm::vec3(0, 10, 0), glm::vec3(1, 1, 1)));
	sceneLights.push_back(std::make_shared<Light>(glm::vec3(0, -10, 0), glm::vec3(1, 1, 1)));
	sceneLights.push_back(std::make_shared<Light>(glm::vec3(10, 10, 0), glm::vec3(1, 1, 1)));
	sceneLights.push_back(std::make_shared<Light>(glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)));
	*/
	sceneLights.push_back(std::make_shared<Light>(glm::vec3(0, 100, 0), glm::vec3(1, 1, 1)));

	std::string model_dir_flat = "res/models/esel_flat/";

	std::string model_name_flat = "esel_flat.obj";

	//std::string model_name_smooth = "esel_smooth.obj";
	std::string model_name_sphere = "sphere.obj";

	loadAndDisplayObject(model_dir_flat + model_name_flat);

	rayObj = setupRayMarchingQuad();
	rayObj->registerTAAShader();

	glm::mat4 projection_matrix = glm::perspective(camera_params.fov, ((float)parameters.windowWidth) / parameters.windowHeight, 0.01f, 1000.0f);
	renderer.setProjection(projection_matrix);

	getLoadableObj("res/models/");

	return true;
}

void SceneController::doFrame()
{
	updateModels();
	float prev = camera_params.cameraPos[0];
	updateCamera();
	float after = camera_params.cameraPos[0];



	float cutoff = 3.1;
	int frame_cutoff = 70;
	/*

	if (parameters.special_bool && prev == 0.0f && after > 0.0f) {
		running_frames = 0.0f;
		num_frames = 0.0f;
	}
	float frames = ImGui::GetIO().Framerate;
	running_frames += frames;
	num_frames++;
	*/
	/*
	if (parameters.special_bool && camera_params.current_frame == frame_cutoff) {
		auto millisec_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

		exportImage(millisec_since_epoch);

	}
	*/
	if (parameters.special_bool && prev < cutoff && after >= cutoff) {
		auto millisec_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

		exportImage(millisec_since_epoch);
	}
	/*
	*/

	if (parameters.current_render_type_index == RASTERIZATION_RENDER_INDEX)
	{
		if (activeObject && activeObject->hasRasterizationObject()) {
			auto object = activeObject->getRasterizationObject();
			if (object->isRenderable()) {
				renderer.render(object.get(), camera, sceneLights);
			}
		}
		else {
			//camera.update();
			// TODO output ?
		}
	}
	else if (parameters.current_render_type_index == VOXEL_INDEX_RENDER_INDEX)
	{
		if (activeObject && activeObject->hasRenderableVoxels() && rayObj->hasVoxelShader()) {
			/*
			std::vector<glm::vec3> vertices = camera.getScreenCoveringQuad();
			rayObj->getArrayBuffer()->updateData(vertices.data(), vertices.size() * 3 * sizeof(float));
			*/
			updateRayMarchQuad();
			auto voxels = activeObject->getVoxels();
			renderer.renderVoxels(rayObj.get(), camera, *voxels);
		}

	}
	else if (parameters.current_render_type_index == OCTREE_VIS_RENDER_INDEX)
	{
		if (activeObject && activeObject->hasRenderableOctreeVisualization()) {
			auto object = activeObject->getOctreeVisObject();
			renderer.renderOctreeVisualization(object.get(), camera);
		}
	}
	else if (parameters.current_render_type_index == OCTREE_RENDER_INDEX)
	{
		if (activeObject && activeObject->hasRenderableOctree() && rayObj->hasValidOctreeShader()) {
			/*
			std::vector<glm::vec3> vertices = camera.getScreenCoveringQuad();
			rayObj->getArrayBuffer()->updateData(vertices.data(), vertices.size() * 3 * sizeof(float));
			*/
			updateRayMarchQuad();
			auto octree = activeObject->getOctree();

			renderer.renderOctree(rayObj.get(), camera, *octree);
		}
	}
}

void SceneController::updateModels()
{
	if (loader.getThreadStatus() == LoaderThreadStatus::FINISHED) {
		loader.receivedResult();
		if (loader.getJobStatus() == LoadJobStatus::SUCCESS) {
			std::shared_ptr<SceneObject> scene_object = sceneObjects[currentlyLoadingPath];

			const std::string rasterization_vert_path = "res/shaders/shader.vert";
			const std::string phong_frag_path = "res/shaders/phong.frag";

			std::shared_ptr<Shader> rasterization_shader = std::make_shared<Shader>(rasterization_vert_path, phong_frag_path);

			if (rasterization_shader->isValid()) {
				std::shared_ptr<RasterizationObject> object
					= loader.registerMeshObject(scene_object->getMeshObject(), rasterization_shader, glm::mat4(1));

				scene_object->registerRasterizationObject(object);
			}
			if (scene_object->hasVoxels()) {
				scene_object->initVoxels();
			}

			if (scene_object->hasOctree()) {
				scene_object->initOctree();
			}

			switchRenderedObject(currentlyLoadingPath);
		}
		else {
			sceneObjects.erase(currentlyLoadingPath);
		}
	}
	// TODO implement UI controls
	exportFrame();
}

void SceneController::updateCamera()
{
	camera.saveViewMat();
	updateCameraPathState(camera_params.doCameraPath);

	float dist = camera_params.camera_dist * camera_params.camera_dist;

	float cos_theta = glm::cos(camera_params.cameraPos[0]);
	float sin_theta = glm::sin(camera_params.cameraPos[0]);
	float cos_phi = glm::cos(camera_params.cameraPos[1]);
	float sin_phi = glm::sin(camera_params.cameraPos[1]);

	const glm::vec3 default_up = glm::vec3(0, 1, 0);
	const glm::vec3 default_view_dir = glm::vec3(0, 0, -1);

	glm::mat4 rotateFlatMatrix = glm::rotate(camera_params.cameraPos[0], default_up);

	glm::vec4 flatSphereCameraPos = rotateFlatMatrix * glm::vec4(default_view_dir, 0);;


	glm::vec3 rotateAxis = glm::cross(glm::vec3(0, 1, 0),
		glm::vec3(flatSphereCameraPos.x, flatSphereCameraPos.y, flatSphereCameraPos.z));

	glm::mat4 rotateMatrix = glm::rotate(camera_params.cameraPos[1], rotateAxis);

	glm::vec4 h_camera_pos = rotateMatrix * flatSphereCameraPos;
	glm::vec4 h_up;
	glm::vec3 lookAt;
	glm::vec3 cameraPos;

	lookAt = glm::vec3(camera_params.lookAtPos[0], camera_params.lookAtPos[1], camera_params.lookAtPos[2]);
	if (camera_params.lock_to_path) {
		Keyframe kf = m_cameraPath.interpolateKeyframeLinear(camera_params.current_frame);
		// Keyframe kf = m_cameraPath.interpolateKeyframeCatmull(camera_params.current_frame);
		cameraPos = kf.position;
		lookAt = cameraPos + kf.rotation;
		glm::vec3 view_dir = glm::normalize(lookAt - cameraPos);
		float angle = glm::asin(view_dir.y);
		glm::vec3 axis = glm::cross(default_up, view_dir);
		glm::mat4 rotateMatrix = glm::rotate(angle, axis);

		h_up = rotateMatrix * glm::vec4(default_up, 0);

		if (camera_params.autoplay && ++camera_params.current_frame > m_cameraPath.getMaxFrame()) {
			camera_params.current_frame = m_cameraPath.getMinFrame();
		}
	}
	else {
		h_up = rotateMatrix * glm::vec4(default_up, 1);
		cameraPos = lookAt + dist * glm::vec3(h_camera_pos.x, h_camera_pos.y, h_camera_pos.z);

	}

	camera.setUpVector(glm::vec3(h_up.x, h_up.y, h_up.z));
	camera.setPosition(cameraPos);
	camera.setLookAt(lookAt);

	if (camera_params.rotateAzimuth) camera_params.cameraPos[0] += camera_params.angle_speed;
	if (camera_params.rotatePolar) camera_params.cameraPos[1] += camera_params.angle_speed;

	if (parameters.special_bool && camera_params.cameraPos[0] > glm::two_pi<float>()) {
		float ave_frames = running_frames / num_frames;
		std::cout << "average frames: " << ave_frames << std::endl;
	}

	camera_params.cameraPos[0] = camera_params.cameraPos[0] > glm::two_pi<float>() ? 0 : camera_params.cameraPos[0];

	camera_params.cameraPos[1] = camera_params.cameraPos[1] > glm::pi<float>() ? -glm::pi<float>() : camera_params.cameraPos[1];
	//std::cout << "angleY: " << angleY << std::endl;


	glm::mat4 projection_matrix = glm::perspective(camera_params.fov, ((float)parameters.windowWidth) / parameters.windowHeight, 0.01f, 1000.0f);
	renderer.setProjection(projection_matrix);
}

void SceneController::lookAtObject()
{
	if (parameters.current_render_type_index == RASTERIZATION_RENDER_INDEX)
	{
		if (activeObject && activeObject->hasRasterizationObject()) {
			auto object = activeObject->getMeshObject();
			glm::vec3 voxel_center = (object.lower + object.higher) / glm::vec3(2);
			camera_params.lookAtPos[0] = voxel_center.x;
			camera_params.lookAtPos[1] = voxel_center.y;
			camera_params.lookAtPos[2] = voxel_center.z;
		}
	}
	else if (parameters.current_render_type_index == VOXEL_INDEX_RENDER_INDEX)
	{
		if (activeObject && activeObject->hasVoxels()) {
			auto voxels = activeObject->getVoxels();
			glm::vec3 voxel_center = (voxels->getLower() + voxels->getHigher()) / glm::vec3(2);
			camera_params.lookAtPos[0] = voxel_center.x;
			camera_params.lookAtPos[1] = voxel_center.y;
			camera_params.lookAtPos[2] = voxel_center.z;
		}

	}
	else if (parameters.current_render_type_index == OCTREE_VIS_RENDER_INDEX || parameters.current_render_type_index == OCTREE_RENDER_INDEX)
	{
		if (activeObject && activeObject->hasOctree()) {
			auto octree = activeObject->getOctree();
			glm::vec3 voxel_center = (octree->getTreeLower() + octree->getTreeHigher()) / glm::vec3(2);
			camera_params.lookAtPos[0] = voxel_center.x;
			camera_params.lookAtPos[1] = voxel_center.y;
			camera_params.lookAtPos[2] = voxel_center.z;
		}
	}
}

void SceneController::reloadShaders()
{
	for (auto& object : sceneObjects) {
		object.second->reloadShaders();
	}
	rayObj->reloadShader();
}

void SceneController::reloadOctreeVis()
{
	activeObject->initOctree();
}

void SceneController::updateCameraPathState(bool state)
{
	if (camera_params.doCameraPath == state) return;
	if (state) {
		camera_params.doCameraPath = true;
		camera_params.current_frame = m_cameraPath.getMinFrame();
	}
	else {
		camera_params.doCameraPath = false;
	}
}

bool SceneController::updateRayMarchQuad()
{
	std::vector<glm::vec3> vertices;
	float width;
	float height;
	camera.getScreenCoveringQuadData(vertices, &width, &height);

	rayObj->getArrayBuffer()->updateData(vertices.data(), vertices.size() * 3 * sizeof(float));
	rayObj->setSizes(width, height);
	return true;
}

bool SceneController::switchRenderedObject(std::string path)
{
	auto res = sceneObjects.find(path);
	if (res == sceneObjects.end()) {
		return false;
	}

	if (activeObject) {
		activeObject->unInitVoxels();
	}

	activeObject = res->second;
	if (activeObject->hasVoxels()) {
		activeObject->initVoxels();
	}
	return true;
}

bool SceneController::removeSceneObject(std::string path, bool evenIfActive)
{
	auto res = sceneObjects.find(path);
	if (res == sceneObjects.end()) {
		return true;
	}

	if (res->second == activeObject) {
		if (!evenIfActive) {
			std::cout << "Error: Can't remove Object from Scene, because it is active and force is not set" << std::endl;
			return false;
		}
		else {
			activeObject->unInitVoxels();
			activeObject = nullptr;
		}
	}
	auto result = sceneObjects.erase(path);
	return result != 0;
}

std::shared_ptr<RayMarchObject> SceneController::setupRayMarchingQuad() {
	std::vector<glm::vec3> vertices = camera.getScreenCoveringQuad();

	std::shared_ptr<VertexArray> va = std::make_shared<VertexArray>();

	std::shared_ptr<ArrayBuffer> verts = std::make_shared<ArrayBuffer>(vertices.data(), vertices.size() * 3 * sizeof(float), true);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	std::string rayVertexShader = "res/shaders/ray_marching.vert";

	std::string rayFragmentShader = "res/shaders/voxels.frag";
	std::shared_ptr<Shader> voxel_shader = std::make_shared<Shader>(rayVertexShader, rayFragmentShader);

	std::string octreeFragmentShader = "res/shaders/octree.frag";
	std::shared_ptr<Shader> octree_shader = std::make_shared<Shader>(rayVertexShader, octreeFragmentShader);

	auto object = std::make_shared<RayMarchObject>(va, verts);
	object->registerVoxelShader(voxel_shader);
	object->registerOctreeShader(octree_shader);

	return object;
}

void SceneController::addKeyframe()
{
	glm::vec3 pos = camera.getPosition();
	glm::vec3 view_dir = camera.getViewDirection();

	m_cameraPath.addKeyframe({ pos, view_dir }, camera_params.add_frame);
}

void SceneController::resetKeyframes()
{
	m_cameraPath.reset();
}

void SceneController::exportImage(unsigned long time)
{
	glm::vec2 dimensions = renderer.getViewPortDimensions();
	const unsigned int channels = 4;
	unsigned int size = 1 * channels * dimensions.x * dimensions.y;

	uint8_t* buffer = new uint8_t[size];

	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, dimensions.x, dimensions.y, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	GLPrintError();
	m_Exporter->exportImage(dimensions.y, dimensions.x, channels, buffer, time);

	delete buffer;
}

void SceneController::exportFrame()
{
	if (parameters.writeVideo) {
		glm::vec2 dimensions = renderer.getViewPortDimensions();
		if (!m_Exporter->videoExportStarted()) {
			m_Exporter->startVideoExport();
		}

		const unsigned int channels = 4;
		unsigned int size = 1 * channels * dimensions.x * dimensions.y;

		uint8_t* buffer = new uint8_t[size];

		glReadBuffer(GL_FRONT);
		glReadPixels(0, 0, dimensions.x, dimensions.y, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
		GLPrintError();
		m_Exporter->registerVideoFrame(dimensions.y, dimensions.x, channels, buffer);

		delete buffer;
	}
	else {
		if (m_Exporter->videoExportStarted()) {
			m_Exporter->endVideoExport();
		}
	}
}

void SceneController::loadAndDisplayObject(std::string object_path)
{
	if (parameters.forceReload) {
		bool result = removeSceneObject(object_path, true);
		if (!result) {
			std::cout << "Error: failed to remove object for forced reload";
			return;
		}
		parameters.forceReload = false;
	}
	else if (switchRenderedObject(object_path)) {
		return;
	}


	std::shared_ptr<SceneObject> scene_object = std::make_shared<SceneObject>();

	bool loadAsync = true;
	if (loadAsync) {
		sceneObjects[object_path] = scene_object;

		bool result = loader.loadSceneObjectAsynch(object_path, scene_object.get());

		if (result) {
			currentlyLoadingPath = object_path;
		}
		else {
			sceneObjects.erase(object_path);
		}
	}
	else {
		bool result = loader.loadSceneObjectSynchronous(object_path, scene_object.get());

		if (result) {
			const std::string rasterization_vert_path = "res/shaders/shader.vert";
			const std::string phong_frag_path = "res/shaders/phong.frag";

			std::shared_ptr<Shader> rasterization_shader = std::make_shared<Shader>(rasterization_vert_path, phong_frag_path);

			if (rasterization_shader->isValid()) {
				std::shared_ptr<RasterizationObject> object
					= loader.registerMeshObject(scene_object->getMeshObject(), rasterization_shader, glm::mat4(1));

				scene_object->registerRasterizationObject(object);
			}

			if (scene_object->hasVoxels()) {
				scene_object->initVoxels();
			}

			if (scene_object->hasOctree()) {
				scene_object->initOctree();
			}

			sceneObjects[object_path] = scene_object;
			switchRenderedObject(object_path);
			//activeObject = scene_object;
		}
	}
}
