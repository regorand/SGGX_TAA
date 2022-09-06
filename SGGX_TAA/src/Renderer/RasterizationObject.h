#pragma once

#include <glm/matrix.hpp>
#include <memory>
#include <vector>

#include "../gl_wrappers/VertexArray.h"
#include "../gl_wrappers/ArrayBuffer.h"
#include "../gl_wrappers/IndexBuffer.h"
#include "../gl_wrappers/Shader.h"
#include "../gl_wrappers/Texture.h"
#include "Material.h"

class RasterizationObject
{
private:
    std::shared_ptr<VertexArray> m_VertexArray;
    std::shared_ptr<IndexBuffer> m_Index_buffer;
    std::shared_ptr<Shader> m_Shader;


    // Hold Materials
    std::vector<Material> m_Materials;

    // This is here so the shared pointer doesn't delete Array Buffers
    std::vector<std::shared_ptr<ArrayBuffer>> m_ArrayBuffers;
    std::vector<std::shared_ptr<Texture>> m_Textures;

    glm::mat4 m_ModelMatrix;
    glm::mat4 m_LocalTransformationMatrix;
public:
    RasterizationObject(std::shared_ptr<VertexArray> vertexArray,
        std::shared_ptr <IndexBuffer> index_buffer, 
        std::shared_ptr <Shader> shader, 
        glm::mat4 model_matrix,
        std::vector<Material> materials = std::vector<Material>());
    ~RasterizationObject();

    void setLocalTransformation(glm::mat4 local_transformation);
    std::shared_ptr<VertexArray> getVertexArray();
    std::shared_ptr<IndexBuffer> getIndexBuffer();
    std::shared_ptr<Shader> getShader();

    std::vector<Material> getMaterials();

    void addArrayBuffer(std::shared_ptr<ArrayBuffer> aBuf);

    void addTexture(std::shared_ptr<Texture> texture);
    std::vector<std::shared_ptr<Texture>> getTextures();

    bool isRenderable();

    void reloadShaders();

    glm::mat4 getModelMatrix();
    glm::mat4 getLocalTransform();
};

