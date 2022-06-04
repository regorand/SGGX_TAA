#include "SceneController.h"
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "../Parameters.h"

SceneController::SceneController()
    : camera(Camera(glm::vec3(0, 0, 1), glm::vec3(0, 0, -1)))
{}

SceneController::~SceneController()
{}


bool SceneController::init()
{
    sceneLights.push_back(std::make_shared<Light>(glm::vec3(10, 0, 0), glm::vec3(1, 1, 1)));
    sceneLights.push_back(std::make_shared<Light>(glm::vec3(0, 10, 0), glm::vec3(1, 1, 1)));
    sceneLights.push_back(std::make_shared<Light>(glm::vec3(0, -10, 0), glm::vec3(1, 1, 1)));
    sceneLights.push_back(std::make_shared<Light>(glm::vec3(10, 10, 0), glm::vec3(1, 1, 1)));
    sceneLights.push_back(std::make_shared<Light>(glm::vec3(100, 10, 0), glm::vec3(1, 1, 1)));
    sceneLights.push_back(std::make_shared<Light>(glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)));

    const std::string vert_path = "res/shaders/shader.vert";
    const std::string phong_frag_path = "res/shaders/phong.frag";
    const std::string GGX_frag_path = "res/shaders/GGX.frag";

    std::string model_dir_flat = "res/models/esel_flat/";
    //std::string model_dir_flat = "res/models/smooth/";
    std::string model_dir_sphere = "res/models/sphere/";
    std::string model_name_flat = "esel_flat.obj";
    //std::string model_name_smooth = "esel_smooth.obj";
    std::string model_name_sphere = "sphere.obj";

    Mesh_Object_t mesh_left;
    Mesh_Object_t mesh_right;

    bool res =  loadObjMesh(model_dir_sphere, model_name_sphere, mesh_left, ShadingType::SMOOTH);
    bool res2 = loadObjMesh(model_dir_sphere, model_name_sphere, mesh_right, ShadingType::SMOOTH);

    if (!res2 || !res) {
        std::cout << "Import of obj Failed" << std::endl;
        return false;
    }

    std::shared_ptr<Shader> shader_left = std::make_shared<Shader>(vert_path, phong_frag_path);
    
    if (shader_left->isValid()) {
        glm::mat4 translation_matrix_left = glm::translate(glm::vec3(2, 0, 0));
        std::shared_ptr<SceneObject> object = registerSceneObject(mesh_left, shader_left, translation_matrix_left);
        sceneObjects.push_back(object);
    }
    
    std::shared_ptr<Shader> shader_right = std::make_shared<Shader>(vert_path, GGX_frag_path);

    if (shader_right->isValid()) {
        glm::mat4 translation_matrix_right = glm::translate(glm::vec3(-2, 0, 0));
        glm::mat4 rot_matrix_right = glm::eulerAngleX(-glm::half_pi<float>());
        glm::mat4 transformation_matrix = translation_matrix_right * rot_matrix_right;
        std::shared_ptr<SceneObject>object_2 = registerSceneObject(mesh_right, shader_right, translation_matrix_right);
        sceneObjects.push_back(object_2);
    }

    //camera = Camera(glm::vec3(0, 0, 1), glm::vec3(0, 0, -1));
    rayObj = setupRayMarchingQuad();
    
    glm::mat4 projection_matrix = glm::perspective(parameters.fov, ((float) parameters.windowWidth) / parameters.windowHeight, 0.01f, 1000.0f);
    renderer.setProjection(projection_matrix);
    return true;
}

void SceneController::doFrame()
{
    updateCamera();
    if (parameters.renderRayMarch && rayObj->getShader()->isValid()) {
        std::vector<glm::vec3> vertices = camera.getScreenCoveringQuad();
        rayObj->getArrayBuffer()->updateData(vertices.data(), vertices.size() * 3 * sizeof(float));
        /*
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
        glEnableVertexAttribArray(0);
        */
        renderer.renderRayMarching(rayObj.get(), camera, sceneLights);
    }
    else {
        glm::mat4 rot = glm::eulerAngleXYZ(parameters.rotation[0], parameters.rotation[1], parameters.rotation[2]);

        //camera.update();
        for (auto& object : sceneObjects) {
            if (object->getShader()->isValid()) {
                object->setLocalTransformation(rot);
                renderer.render(object.get(), camera, sceneLights);
            }
        }
    }
}

void SceneController::updateCamera()
{
    
    camera.setPosition(glm::vec3(parameters.camera_dist * glm::sin(angleY), 0, -parameters.camera_dist * glm::cos(angleY)));
    camera.setLookAt(glm::vec3(0, 0, 0));
    if (parameters.rotateCamera) {
        angleY += angle_speed_Y;
    }
    angleY = angleY > glm::two_pi<float>() ? 0 : angleY;
    //std::cout << "angleY: " << angleY << std::endl;
}

void SceneController::reloadShaders()
{
    for (auto& object : sceneObjects) {
        object->reloadShaders();
    }
    rayObj->reloadShader();
}

std::shared_ptr<SceneObject> registerSceneObject(Mesh_Object_t &source, std::shared_ptr<Shader> shader, glm::mat4 model_matrix) {
    shader->bind();
    std::shared_ptr<VertexArray> va = std::make_shared<VertexArray>();


    std::shared_ptr<ArrayBuffer> vertices = std::make_shared<ArrayBuffer>(source.vertices.data(), source.vertices.size() * sizeof(float));
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);


    std::shared_ptr<ArrayBuffer> normals = std::make_shared<ArrayBuffer>(source.normals.data(), source.normals.size() * sizeof(float));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(1);

    //if (source.tex_coords.size() != 0) {
        std::shared_ptr<ArrayBuffer> tex_coords = std::make_shared<ArrayBuffer>(source.tex_coords.data(), source.tex_coords.size() * sizeof(float));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
        glEnableVertexAttribArray(2);

        /*/
    if (source.colors.size() != 0) {
        std::shared_ptr<ArrayBuffer> colors = std::make_shared<ArrayBuffer>(source.colors.data(), source.colors.size() * sizeof(float));
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glEnableVertexAttribArray(3);
        object->addArrayBuffer(colors);
    }*/
        std::shared_ptr<IndexBuffer> ib = std::make_shared<IndexBuffer>(source.indices.data(), source.indices.size());
        std::shared_ptr<SceneObject> object = std::make_shared<SceneObject>(va, ib, shader, model_matrix, source.materials);

    object->addArrayBuffer(vertices);
    object->addArrayBuffer(normals);
    object->addArrayBuffer(tex_coords);

    for (int i = 0; i < object->getMaterials().size(); i++) {
        auto mat = object->getMaterials()[i];
        if (mat.diffuse_texname != "") {
            std::shared_ptr<Texture> tex = std::make_shared<Texture>(mat.diffuse_texname);
            object->addTexture(tex);
        }
    }
    shader->unbind();
    return object;
}

std::shared_ptr<RayMarchObject> SceneController::setupRayMarchingQuad() {
    std::vector<glm::vec3> vertices = camera.getScreenCoveringQuad();
    
    std::shared_ptr<VertexArray> va = std::make_shared<VertexArray>();

    std::shared_ptr<ArrayBuffer> verts = std::make_shared<ArrayBuffer>(vertices.data(), vertices.size() * 3 * sizeof(float));
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    std::string rayVertexShader = "res/shaders/ray_marching.vert";

    //std::string rayFragmentShader = "res/shaders/ray_marching.frag";
    std::string rayFragmentShader = "res/shaders/ray_trace_bounding.frag";
    //std::string rayFragmentShader = "res/shaders/sphere_tracing.frag";

    std::shared_ptr<Shader> shader = std::make_shared<Shader>(rayVertexShader, rayFragmentShader);

    return std::make_shared<RayMarchObject>(va, verts, shader);
}