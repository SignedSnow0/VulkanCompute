#include "AssetManager.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "Core/Logger.h"

std::shared_ptr<Mesh> processMesh(const aiScene *scene, const aiMesh *mesh) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    vertices.reserve(mesh->mNumVertices);
    indices.reserve(mesh->mNumFaces * 3);

    LOG_DEBUG("Processing mesh with {} vertices and {} indices",
              mesh->mNumVertices, mesh->mNumFaces * 3);

    for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex{};

        vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y,
                                    mesh->mVertices[i].z);

        vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y,
                                  mesh->mNormals[i].z);

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

    return std::make_shared<Mesh>(vertices, indices);
}

void processNode(const aiScene *scene, const aiNode *node,
                 std::shared_ptr<Scene> &outScene) {
    for (uint32_t i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        auto processedMesh = processMesh(scene, mesh);
        outScene->AddMesh(processedMesh);
    }

    for (uint32_t i = 0; i < node->mNumChildren; i++) {
        processNode(scene, node->mChildren[i], outScene);
    }
}

MeshRenderer::MeshRenderer(const std::shared_ptr<VulkanManager> &vulkanManager,
                           const std::shared_ptr<Mesh> &mesh)
    : mVulkanManager(vulkanManager), mMesh(mesh) {
    mVertexBuffer = std::make_unique<Buffer<Vertex>>(
        mVulkanManager, mMesh->mVertices.data(),
        mMesh->mVertices.size() * sizeof(Vertex),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    mIndexBuffer = std::make_unique<Buffer<uint32_t>>(
        mVulkanManager, mMesh->mIndices.data(),
        mMesh->mIndices.size() * sizeof(uint32_t),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
    : mVertices(std::move(vertices)), mIndices(std::move(indices)) {}

std::shared_ptr<Scene> AssetManager::LoadScene(const std::string &filepath) {
    Assimp::Importer importer;

    const aiScene *scene = importer.ReadFile(
        filepath, aiProcess_GenSmoothNormals | aiProcess_PreTransformVertices | 
        aiProcess_JoinIdenticalVertices | aiProcess_Triangulate);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !scene->mMeshes) {
        LOG_WARNING("Failed to load model: {}", importer.GetErrorString());
        return nullptr;
    }

    auto newScene = std::make_shared<Scene>();

    for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[i];
        auto processedMesh = processMesh(scene, mesh);
        newScene->AddMesh(processedMesh);
    }

    return newScene;
}

void Scene::AddMesh(const std::shared_ptr<Mesh> &mesh) {
    mMeshes.push_back(mesh);
}