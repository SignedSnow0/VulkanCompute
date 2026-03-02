#include "AssetManager.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "Core/Logger.h"

std::shared_ptr<Mesh> AssetManager::LoadMesh(const std::string& filepath, const glm::mat4& modelMatrix) {
    Assimp::Importer importer;

    const aiScene *scene = importer.ReadFile(
        filepath, aiProcess_GenSmoothNormals | aiProcess_PreTransformVertices | 
        aiProcess_JoinIdenticalVertices | aiProcess_Triangulate);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !scene->mMeshes) {
        LOG_WARNING("Failed to load mesh: {}", importer.GetErrorString());
        return nullptr;
    }

    auto newMesh = std::make_shared<Mesh>();
    ProcessNode(scene, scene->mRootNode, newMesh.get(), modelMatrix);

    return newMesh;
}

void AssetManager::ProcessNode(const aiScene *scene, const aiNode *node,
    Mesh* outMesh, const glm::mat4& modelMatrix) {
    std::vector<Triangle> triangles;
    for (uint32_t i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        auto meshTriangles = AssetManager::ProcessMesh(scene, mesh, modelMatrix);
        triangles.insert(triangles.end(), meshTriangles.begin(), meshTriangles.end());
    }
    outMesh->mTriangles = std::move(triangles);

    for (uint32_t i = 0; i < node->mNumChildren; i++) {
        auto subMesh = Mesh();
        AssetManager::ProcessNode(scene, node->mChildren[i], &subMesh, modelMatrix);
        outMesh->subMeshes.push_back(std::move(subMesh));
    }
}

std::vector<Triangle> AssetManager::ProcessMesh(const aiScene* scene,
                                                const aiMesh* mesh,
                                                const glm::mat4& modelMatrix) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    vertices.reserve(mesh->mNumVertices);

    LOG_DEBUG("Processing mesh with {} vertices",
              mesh->mNumVertices);

    for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex{};

        vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y,
            mesh->mVertices[i].z);

        vertex.position = glm::vec3(modelMatrix * glm::vec4(vertex.position, 1.0f));
        vertex.normal = glm::vec3(modelMatrix * glm::vec4(mesh->mNormals[i].x, mesh->mNormals[i].y,
            mesh->mNormals[i].z, 0.0f));

        if (mesh->mTextureCoords[0]) {
            vertex.uv = glm::vec2(mesh->mTextureCoords[0][i].x,
                                  mesh->mTextureCoords[0][i].y);
        } else {
            vertex.uv = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (uint32_t j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    std::vector<Triangle> triangles;
    triangles.reserve(indices.size() / 3);
    for (size_t i = 0; i < indices.size(); i += 3) {
        Triangle tri;
        tri.V0 = vertices[indices[i]].position;
        tri.V1 = vertices[indices[i + 1]].position;
        tri.V2 = vertices[indices[i + 2]].position;
        triangles.push_back(tri);
    }

    return triangles;
}