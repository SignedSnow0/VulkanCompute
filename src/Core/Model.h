#pragma once

#include <vector>
#include <string>
#include <memory>

#include <glm/glm.hpp>

#include "App/UBOs.h"

class AssetManager;

class Mesh {
public:
    [[nodiscard]] const std::vector<Triangle>& Triangles() const {
        return mTriangles;
    }

    [[nodiscard]] const std::vector<Mesh>& SubMeshes() const {
        return subMeshes;
    }
    
private:
    std::vector<Triangle> mTriangles;
    std::vector<Mesh> subMeshes;
    
    friend class AssetManager;
};

class Model {
public:
    Model(const std::string& path, const Material& material, const glm::mat4& modelMatrix = glm::mat4(1.0f));

    [[nodiscard]] inline const Mesh& GetMesh() const { return *mMesh; }
    [[nodiscard]] inline const Material& GetMaterial() const { return mMaterial; }
    [[nodiscard]] inline const glm::mat4& GetModelMatrix() const { return mModelMatrix; }

private:
    std::shared_ptr<Mesh> mMesh;
    Material mMaterial;

    glm::mat4 mModelMatrix;
};