#include "SceneController.h"
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "../Parameters.h"

SceneController::SceneController()
    : camera(Camera(glm::vec3(0, 0, 1), glm::vec3(0, 0, -1))), m_voxels(new VoxelGrid(100))
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
    std::string model_dir_tree = "res/models/chestnut/";

    //std::string model_dir_flat = "res/models/smooth/";
    std::string model_dir_sphere = "res/models/sphere/";
    std::string model_name_flat = "esel_flat.obj";
    std::string model_name_tree = "AL05a.obj";

    //std::string model_name_smooth = "esel_smooth.obj";
    std::string model_name_sphere = "sphere.obj";

    Mesh_Object_t mesh_left;
    Mesh_Object_t mesh_right;

    bool res =  loadObjMesh(model_dir_flat, model_name_flat, mesh_left, ShadingType::FLAT);
    //bool res = loadObjMesh(model_dir_tree, model_name_tree, mesh_left, ShadingType::FLAT);

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

    initVoxels(mesh_left);

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
        //renderer.renderRayMarching(rayObj.get(), camera, sceneLights);
        VoxelGrid *vg = m_voxels.get();
        renderer.renderVoxels(rayObj.get(), camera, *vg);
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

bool SceneController::initVoxels(Mesh_Object_t obj)
{
    std::cout << "Initializing Voxels" << std::endl;
    m_voxels->setLower(obj.lower);
    m_voxels->setHigher(obj.higher);
    float voxelSize = m_voxels->getVoxelSize();
    auto dimension = m_voxels->getDimension();
    /*
    for (uint16_t x = 0; x < dimension; x++) {
        for (uint16_t y = 0; y < dimension; y++) {
            for (uint16_t z = 0; z < dimension; z++) {
                glm::u16vec3 idx_vec = glm::u16vec3(x, y, z);
                uint16_t dist = glm::floor(glm::length(glm::vec3(idx_vec) - glm::vec3(dimension / 2)));
                float len = glm::sqrt(idx_vec.x* idx_vec.x + idx_vec.y * idx_vec.y + idx_vec.z * idx_vec.z);

                float lower = 5;
                if (dist > lower && dist <= lower + 1) {
                    m_voxels->setVoxel(idx_vec, { 1 });
                }
                else {
                    m_voxels->setVoxel(idx_vec, { 0 });
                }
            }
        }
    }
    */
    unsigned int count = 0;
    for (uint32_t idx = 0; idx < obj.indices.size(); idx += 3) {
        if (idx % 3000 == 0) {
            std::cout << "voxelizing triangle #" << idx / 3 << std::endl;
        }
        glm::vec3 v1 = glm::vec3(obj.vertices[3 * obj.indices[idx]],     obj.vertices[3 * obj.indices[idx] + 1],     obj.vertices[3 * obj.indices[idx] + 2]);
        glm::vec3 v2 = glm::vec3(obj.vertices[3 * obj.indices[idx + 1]], obj.vertices[3 * obj.indices[idx + 1] + 1], obj.vertices[3 * obj.indices[idx + 1] + 2]);
        glm::vec3 v3 = glm::vec3(obj.vertices[3 * obj.indices[idx + 2]], obj.vertices[3 * obj.indices[idx + 2] + 1], obj.vertices[3 * obj.indices[idx + 2] + 2]);

        glm::vec3 n1 = glm::vec3(obj.normals[3 * obj.indices[idx]],     obj.normals[3 * obj.indices[idx] + 1],     obj.normals[3 * obj.indices[idx] + 2]);
        glm::vec3 n2 = glm::vec3(obj.normals[3 * obj.indices[idx + 1]], obj.normals[3 * obj.indices[idx + 1] + 1], obj.normals[3 * obj.indices[idx + 1] + 2]);
        glm::vec3 n3 = glm::vec3(obj.normals[3 * obj.indices[idx + 2]], obj.normals[3 * obj.indices[idx + 2] + 1], obj.normals[3 * obj.indices[idx + 2] + 2]);

        glm::vec3 center = (v1 + v2 + v3) / glm::vec3(3);
        glm::vec3 normal = (n1 + n2 + n3) / glm::vec3(3);
 
        glm::u16vec3 idx_vec = glm::u16vec3(glm::floor((center - m_voxels->getLower()) / m_voxels->getVoxelSize()));
        if (idx_vec.x >= 0 && idx_vec.x < dimension
            && idx_vec.y >= 0 && idx_vec.y < dimension
            && idx_vec.z >= 0 && idx_vec.z < dimension) {
            m_voxels->setVoxel(idx_vec, { 1, normal.x, normal.y, normal.z });
        }
    }


    /*
    for (uint32_t idx = 0; idx < obj.indices.size(); idx+=3) {
        //std::cout << "Doing triangle idx: " << idx << "\ttriangle#: " << idx / 3 << std::endl;
        glm::vec3 v1 = glm::vec3(obj.vertices[obj.indices[idx]], obj.vertices[obj.indices[idx] + 1], obj.vertices[obj.indices[idx] + 2]);
        glm::vec3 v2 = glm::vec3(obj.vertices[obj.indices[idx + 1]], obj.vertices[obj.indices[idx + 1] + 1], obj.vertices[obj.indices[idx + 1] + 2]);
        glm::vec3 v3 = glm::vec3(obj.vertices[obj.indices[idx + 2]], obj.vertices[obj.indices[idx + 2] + 1], obj.vertices[obj.indices[idx + 2] + 2]);

        glm::vec3 normal = glm::cross(v2 - v1, v3 - v1);
        float d = glm::dot(v1, normal);

        for (uint16_t x = 0; x < dimension; x++) {
            for (uint16_t y = 0; y < dimension; y++) {
                for (uint16_t z = 0; z < dimension; z++) {
                    glm::u16vec3 idx_vec = glm::u16vec3(x, y, z);
                    glm::vec3 center = m_voxels->getCenterOfVoxel(idx_vec);
                    float dist_to_triangles = 2 * voxelSize;
                    float dot = glm::dot(normal, glm::vec3(1, 0, 0));
                    if (dot != 0) {
                        float t = d - glm::dot(center, normal) / dot;
                        dist_to_triangles = glm::min(dist_to_triangles, glm::abs(t));
                    }

                    dot = glm::dot(normal, glm::vec3(0, 1, 0));
                    if (dot != 0) {
                        float t = d - glm::dot(center, normal) / dot;
                        dist_to_triangles = glm::min(dist_to_triangles, glm::abs(t));
                    }

                    dot = glm::dot(normal, glm::vec3(0, 0, 1));
                    if (dot != 0) {
                        float t = d - glm::dot(center, normal) / dot;
                        dist_to_triangles = glm::min(dist_to_triangles, glm::abs(t));
                    }

                    if (dist_to_triangles < glm::sqrt(3 * voxelSize * voxelSize)) {
                        m_voxels->setVoxel(idx_vec, { 1 });
                    }
                    else {
                        m_voxels->setVoxel(idx_vec, { 0 });
                    }
                }
            }
        }
    }
    */
    m_voxels->initBuffers();

    return true;
}

void SceneController::updateCamera()
{
    float dist = parameters.camera_dist * parameters.camera_dist;
    camera.setPosition(glm::vec3(dist * glm::sin(angleY), 0, -dist * glm::cos(angleY)));
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