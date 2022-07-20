#include "SceneController.h"
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "../Parameters.h"

#include "../utils/file_utils.h"

SceneController::SceneController()
    : camera(Camera(glm::vec3(0, 0, 1), glm::vec3(0, 0, -1))), m_voxels(nullptr)
{}

SceneController::~SceneController()
{}


bool SceneController::init()
{
    /*
    sceneLights.push_back(std::make_shared<Light>(glm::vec3(10, 0, 0), glm::vec3(1, 1, 1)));
    sceneLights.push_back(std::make_shared<Light>(glm::vec3(0, 10, 0), glm::vec3(1, 1, 1)));
    sceneLights.push_back(std::make_shared<Light>(glm::vec3(0, -10, 0), glm::vec3(1, 1, 1)));
    sceneLights.push_back(std::make_shared<Light>(glm::vec3(10, 10, 0), glm::vec3(1, 1, 1)));
    sceneLights.push_back(std::make_shared<Light>(glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)));
    */
    sceneLights.push_back(std::make_shared<Light>(glm::vec3(0, 100, 0), glm::vec3(1, 1, 1)));

    const std::string vert_path = "res/shaders/shader.vert";
    const std::string phong_frag_path = "res/shaders/phong.frag";
    const std::string GGX_frag_path = "res/shaders/GGX.frag";

    std::string model_dir_flat = "res/models/esel_flat/";
    std::string model_dir_tree = "res/models/chestnut/";
    std::string model_dir_tree2 = "res/models/tree/";

    //std::string model_dir_flat = "res/models/smooth/";
    std::string model_dir_sphere = "res/models/sphere/";
    std::string model_name_flat = "esel_flat.obj";
    std::string model_name_tree = "AL05a.obj";
    std::string model_name_tree2 = "Tree.obj";

    //std::string model_name_smooth = "esel_smooth.obj";
    std::string model_name_sphere = "sphere.obj";

    loadAndDisplayObject(model_dir_flat + model_name_flat);

    rayObj = setupRayMarchingQuad();

    glm::mat4 projection_matrix = glm::perspective(camera_params.fov, ((float)parameters.windowWidth) / parameters.windowHeight, 0.01f, 1000.0f);
    renderer.setProjection(projection_matrix);

    getLoadableObj("res/models/");

    return true;
}

void SceneController::doFrame()
{
    updateModels();
    updateCamera();
    if (parameters.active_render_type == "Rasterization") 
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
    else if (parameters.active_render_type == "Voxels")
    {
        if (activeObject && activeObject->hasRenderableVoxels()) {
            std::vector<glm::vec3> vertices = camera.getScreenCoveringQuad();
            rayObj->getArrayBuffer()->updateData(vertices.data(), vertices.size() * 3 * sizeof(float));
            auto voxels = activeObject->getVoxels();
            renderer.renderVoxels(rayObj.get(), camera, *voxels);
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
            scene_object->initVoxels();

            switchRenderedObject(currentlyLoadingPath);
        }
        else {
            sceneObjects.erase(currentlyLoadingPath);
        }
    }
}

void SceneController::updateCamera()
{
    float dist = camera_params.camera_dist * camera_params.camera_dist;

    float cos_theta = glm::cos(camera_params.cameraPos[0]);
    float sin_theta = glm::sin(camera_params.cameraPos[0]);
    float cos_phi   = glm::cos(camera_params.cameraPos[1]);
    float sin_phi   = glm::sin(camera_params.cameraPos[1]);


    glm::vec3 sphereCameraPos = glm::vec3(
         glm::cos(camera_params.cameraPos[1]) * glm::sin(camera_params.cameraPos[0]),
         glm::sin(camera_params.cameraPos[1]),
        -glm::cos(camera_params.cameraPos[1]) * glm::cos(camera_params.cameraPos[0])
    );


    glm::vec3 lookAt = glm::vec3(camera_params.lookAtPos[0], camera_params.lookAtPos[1], camera_params.lookAtPos[2]);
    glm::vec3 cameraPos = lookAt + dist * sphereCameraPos;
    camera.setPosition(cameraPos);
    camera.setLookAt(lookAt);

    /*
    if (activeObject && activeObject->hasVoxels()) {
        auto voxels = activeObject->getVoxels();
        glm::vec3 voxel_center = (voxels->getLower() + voxels->getHigher()) / glm::vec3(2);
        //camera.setLookAt(voxel_center);
        camera.setLookAt(glm::vec3(0, 0, 0));
    }
    else {
        camera.setLookAt(glm::vec3(0, 0, 0));
    }
    */

    if (camera_params.rotateAzimuth) camera_params.cameraPos[0] += camera_params.angle_speed;
    if (camera_params.rotatePolar) camera_params.cameraPos[1] += camera_params.angle_speed;

    camera_params.cameraPos[0] = camera_params.cameraPos[0] > glm::two_pi<float>() ? 0 : camera_params.cameraPos[0];

    camera_params.cameraPos[1] = camera_params.cameraPos[1] > glm::half_pi<float>() ? -glm::half_pi<float>() : camera_params.cameraPos[1];
    //std::cout << "angleY: " << angleY << std::endl;

    glm::mat4 projection_matrix = glm::perspective(camera_params.fov, ((float)parameters.windowWidth) / parameters.windowHeight, 0.01f, 1000.0f);
    renderer.setProjection(projection_matrix);
}

void SceneController::lookAtObject()
{
    // TODO could also look for mesh object in active object
    // might catch some extra use cases
    if (activeObject && activeObject->hasVoxels()) {
        auto voxels = activeObject->getVoxels();
        glm::vec3 voxel_center = (voxels->getLower() + voxels->getHigher()) / glm::vec3(2);
        camera_params.lookAtPos[0] = voxel_center.x;
        camera_params.lookAtPos[1] = voxel_center.y;
        camera_params.lookAtPos[2] = voxel_center.z;
    }
}

void SceneController::reloadShaders()
{
    for (auto& object : sceneObjects) {
        object.second->reloadShaders();
    }
    rayObj->reloadShader();
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

    //std::string rayFragmentShader = "res/shaders/ray_marching.frag";
    //std::string rayFragmentShader = "res/shaders/ray_trace_bounding.frag";
    //std::string rayFragmentShader = "res/shaders/sphere_tracing.frag";

    std::string rayFragmentShader = "res/shaders/voxels.frag";

    std::shared_ptr<Shader> shader = std::make_shared<Shader>(rayVertexShader, rayFragmentShader);

    return std::make_shared<RayMarchObject>(va, verts, shader);
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

            scene_object->initVoxels();

            sceneObjects[object_path] = scene_object;
            switchRenderedObject(object_path);
            //activeObject = scene_object;
        }
    }
}

// Octree Pseudocode idea
/*

struct node {
    uint32_t type_and_index;
};

struct inner_node {
    uint32_t children[8];
};

struct leaf {
    float value1;
    float value2;
};

struct octree {
    glm::vec3 lower;
    glm::vec3 higher;
    uint16_t max_depth;
    node root;
};

std::vector<node> nodes;
std::vector<inner_node> inner_nodes;
std::vector<leaf> leaves;

*/
