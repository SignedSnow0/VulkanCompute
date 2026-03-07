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
    [[nodiscard]] std::vector<Triangle>& Triangles() {
        return mTriangles;
    }

    [[nodiscard]] const std::vector<Mesh>& SubMeshes() const {
        return subMeshes;
    }
    [[nodiscard]] std::vector<Mesh>& SubMeshes() {
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

    bool GetUpdate() const { return mUpdate; }
    void SetUpdate(bool update) { mUpdate = update; }

    [[nodiscard]] inline const Mesh& GetMesh() const { return *mMesh; }
    [[nodiscard]] inline const Material& GetMaterial() const { return mMaterial; }
    [[nodiscard]] inline const glm::mat4& GetModelMatrix() const { return mModelMatrix; }

    void SetModelMatrix(const glm::mat4& modelMatrix) {
        if (modelMatrix != mModelMatrix) {
            mModelMatrix = modelMatrix;
            mUpdate = true;
        }
    }
    
private:
    std::shared_ptr<Mesh> mMesh;
    Material mMaterial;

    glm::mat4 mModelMatrix;
    bool mUpdate{ false };
};