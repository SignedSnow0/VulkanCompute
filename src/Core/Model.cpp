#include "Model.h"

#include <map>

#include "Core/AssetManager.h"

static std::map<std::string, std::shared_ptr<Mesh>> sMeshCache;

Model::Model(const std::string& path, const Material& material, const glm::mat4& modelMatrix)
    : mMaterial(material), mModelMatrix(modelMatrix) {
    if (sMeshCache.contains(path)) {
        mMesh = sMeshCache[path];
        return;
    }

    mMesh = AssetManager::LoadMesh(path);
    sMeshCache[path] = mMesh;

    mUpdate = modelMatrix != glm::mat4(1.0f);
}