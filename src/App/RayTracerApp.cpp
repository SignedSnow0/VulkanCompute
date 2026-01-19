#include "RayTracerApp.h"
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <time.h>
#include <imgui.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

static constexpr glm::vec3 up = glm::vec3(0, 1, 0);

RayTracerApp::RayTracerApp()
    : VulkanComputeApp(1920, 1080, "Vulkan Ray Tracer", 1) {
    std::random_device rd;
    mRandomGenerator = std::mt19937(rd());
    mRandomDistribution = std::uniform_int_distribution<uint32_t>();

    mShader = std::shared_ptr<Shader>(
        Shader::Create(mVulkanManager, "assets/shaders/RayTracer.comp",
                       ShaderStage::Compute, mSurface->ImageCount()));
    mPipeline = std::make_shared<ComputePipeline>(mVulkanManager, mShader);

    mSceneData = std::make_shared<UniformBuffer<SceneData>>(
        mVulkanManager);
    mCamera = std::make_shared<UniformBuffer<Camera>>(
        mVulkanManager);
}

RayTracerApp::~RayTracerApp() = default;

void RayTracerApp::OnStart() {
    glm::vec3 translation = glm::vec3(0, -.5, -.5);
    glm::vec3 rotation = glm::vec3(-90, -90, 0);
    glm::vec3 scale = glm::vec3(0.4);

    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), translation) * glm::toMat4(glm::quat{ glm::radians(rotation) }) * glm::scale(glm::mat4(1.0f), scale);
    mScene = AssetManager::LoadScene("assets/models/viking_room/viking_room.obj", modelMatrix);
    for (auto& mesh : mScene->GetMeshes()) {
        mMeshes.push_back(MeshRenderer{ mVulkanManager, mesh });
        mAABBs.push_back(CalculateAABB(mesh));
    }

    Material material;
    material.color = { 1, .64, .22 };
    material.emission_color = { 0, 0, 0, 0};
    material.metalness = .4;
    mMaterials.push_back(material);

    BuildScene();

    Sphere lightSphere;
    lightSphere.position = { -4, 5, -10 };
    lightSphere.radius = 5;
    lightSphere.materialIndex = mMaterials.size();
    mSpheres.push_back(lightSphere);

    material.color = { 0, 0, 0 };
    material.emission_color = { 1, 1, 1, 1 };
    material.metalness = 0;
    mMaterials.push_back(material);

    Plane groundPlane;
    groundPlane.position = { 0, -1, 0 };
    groundPlane.normal = { 0, 1, 0 };
    groundPlane.materialIndex = mMaterials.size();
    mPlanes.push_back(groundPlane);

    material.color = { .45, .67, .44 };
    material.emission_color = {0, 0, 0, 0};
    material.metalness = 0;
    mMaterials.push_back(material);

    mSpheresBuffer = std::make_shared<Buffer<Sphere>>(
        mVulkanManager, mSpheres.data(), sizeof(Sphere) * mSpheres.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    mPlanesBuffer = std::make_shared<Buffer<Plane>>(
        mVulkanManager, mPlanes.data(), sizeof(Plane) * mPlanes.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    mMaterialsBuffer = std::make_shared<Buffer<Material>>(
        mVulkanManager, mMaterials.data(), sizeof(Material) * mMaterials.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    mAABBsBuffer = std::make_shared<Buffer<AABB>>(
        mVulkanManager, mAABBs.data(), sizeof(AABB) * mAABBs.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
}

void RayTracerApp::OnUpdate(float dt) {
    static SceneData sceneData{ 0, 0, 8 };
    static Camera camera{ glm::vec3{ 0, .5, .99 }, glm::vec3{ 0, 0, -1 } };
    static uint32_t frameIndex = 0;

    sceneData.seed = mRandomDistribution(mRandomGenerator);

    if (frameIndex % 2 == 0 && frameIndex >= 2) {
        sceneData.numFrames++;
    }
    frameIndex++;

    bool oldGuiState = mShowGui;
    if (!mShowGui) {
        if (MoveCamera(camera, dt)) {
            sceneData.numFrames = 0;
            frameIndex = 0;
        }
    } else {
        RenderGui(sceneData, dt);
    }

    if (mWindow.IsKeyPressed(GLFW_KEY_G)) {
        mShowGui = !mShowGui;
    }

    if (oldGuiState && !mShowGui) {
        sceneData.numFrames = 0;
        frameIndex = 0;
        
        UpdateBuffers();
    }

    mSceneData->UpdateData(sceneData);
    mCamera->UpdateData(camera);
}


void RayTracerApp::OnRender(float dt,
    std::shared_ptr<CommandBuffer> commandBuffer) {
    BindUniformBuffers(commandBuffer);
    BindStorageBuffers(commandBuffer);

    mShader->BindSurfaceAsImage(
        mSurface, "gWindow",
        commandBuffer->CurrentBufferIndex());

    if (mShowGui) {
        return;
    }

    mPipeline->Dispatch(
        commandBuffer, (mSurface->Extent().width + 7) / 8,
        (mSurface->Extent().height + 7) / 8,
        1);
}

void RayTracerApp::OnStop() {}
    
void RayTracerApp::BuildScene() {
    Sphere sphere;
    sphere.position = { -1, 0, -2 };
    sphere.radius = 1;
    sphere.materialIndex = mMaterials.size();
    mSpheres.push_back(sphere);

    Material material;
    material.color = { .55, .89, 1};
    material.emission_color = { 0, 0, 0, 0 };
    material.metalness = .4;
    mMaterials.push_back(material);

    sphere.position = { 1, 0, -2 };
    sphere.radius = 1;
    sphere.materialIndex = mMaterials.size();
    mSpheres.push_back(sphere);

    material.color = { 1, .34, .34 };
    material.emission_color = { 0, 0, 0, 0 };
    material.metalness = .95;
    mMaterials.push_back(material);

    sphere.position = { -4, 5, -10 };
    sphere.radius = 5;
    sphere.materialIndex = mMaterials.size();
    mSpheres.push_back(sphere);

    material.color = {0, 0, 0};
    material.emission_color = {1, 1, 1, 1};
    material.metalness = 0;
    mMaterials.push_back(material);
}

void RayTracerApp::RenderGui(SceneData& sceneData, float dt) {
    int i = 0;

    ImGui::Begin("Scene settings");
    {
        ImGui::InputInt("Max bounces", reinterpret_cast<int*>(&sceneData.maxBounces));
        if (sceneData.maxBounces < 0) {
            sceneData.maxBounces = 0;
        }
    }
    ImGui::End();
    ImGui::Begin("Mesh");
    {
        ImGui::PushID(static_cast<int>(i));
        ImGui::Text("Mesh %d", static_cast<int>(i));
        ImGui::ColorEdit3("Color", &mMaterials[0].color.x);
        ImGui::ColorEdit3("Emission Color", &mMaterials[0].emission_color.x);
        ImGui::DragFloat("Emission strength", &mMaterials[0].emission_color.w, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Metalness", &mMaterials[0].metalness, 0.01f, 0.0f, 1.0f);
        ImGui::PopID();
    }
    ImGui::End();

    ImGui::Begin("Spheres");
    for (auto& sphere : mSpheres) {
        ImGui::PushID(static_cast<int>(i));
        ImGui::Text("Sphere %d", static_cast<int>(i));
        ImGui::DragFloat3("Position", &sphere.position.x, 0.1f);
        ImGui::DragFloat("Radius", &sphere.radius, 0.1f, 0.1f, 100.0f);
        ImGui::ColorEdit3("Color", &mMaterials[sphere.materialIndex].color.x);
        ImGui::ColorEdit3("Emission Color", &mMaterials[sphere.materialIndex].emission_color.x);
        ImGui::DragFloat("Emission strength", &mMaterials[sphere.materialIndex].emission_color.w, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Metalness", &mMaterials[sphere.materialIndex].metalness, 0.01f, 0.0f, 1.0f);
        ImGui::Separator();
        if (ImGui::Button("New Sphere")) {
            Sphere newSphere;
            newSphere.position = { 0, 0, 0 };
            newSphere.radius = 1;
            newSphere.materialIndex = static_cast<int>(mMaterials.size());
            mSpheres.push_back(newSphere);

            Material newMaterial;
            newMaterial.color = { 1, 1, 1 };
            newMaterial.emission_color = { 0, 0, 0, 0 };
            newMaterial.metalness = 0;
            mMaterials.push_back(newMaterial);
        }
        ImGui::PopID();

        i++;
    }
    ImGui::End();

    ImGui::Begin("Planes");
    i = 0;
    for (auto& plane : mPlanes) {
        ImGui::PushID(static_cast<int>(i));
        ImGui::Text("Plane %d", static_cast<int>(i));
        ImGui::DragFloat3("Position", &plane.position.x, 0.1f);
        ImGui::DragFloat3("Normal", &plane.normal.x, 0.1f);
        ImGui::ColorEdit3("Color", &mMaterials[plane.materialIndex].color.x);
        ImGui::ColorEdit3("Emission Color", &mMaterials[plane.materialIndex].emission_color.x);
        ImGui::DragFloat("Emission strength", &mMaterials[plane.materialIndex].emission_color.w, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Metalness", &mMaterials[plane.materialIndex].metalness, 0.01f, 0.0f, 1.0f);
        ImGui::Separator();
        if (ImGui::Button("New Plane")) {
            Plane newPlane;
            newPlane.position = { 0, 0, 0 };
            newPlane.normal = { 0, 1, 0 };
            newPlane.materialIndex = static_cast<int>(mMaterials.size());
            mPlanes.push_back(newPlane);

            Material newMaterial;
            newMaterial.color = { 1, 1, 1 };
            newMaterial.emission_color = { 0, 0, 0, 0 };
            newMaterial.metalness = 0;
            mMaterials.push_back(newMaterial);
        }
        ImGui::PopID();

        i++;
    }
    ImGui::End();
}

void RayTracerApp::BindUniformBuffers(const std::shared_ptr<CommandBuffer>& commandBuffer) {
    mShader->BindUniformBuffer(
        *mSceneData, "gSceneData",
        commandBuffer
            ->CurrentBufferIndex());

    mShader->BindUniformBuffer(
        *mCamera, "gCamera", commandBuffer->CurrentBufferIndex());
}

void RayTracerApp::BindStorageBuffers(const std::shared_ptr<CommandBuffer>& commandBuffer) {
    mShader->BindBuffer(*mSpheresBuffer, "gSphereBuffer",
                        commandBuffer->CurrentBufferIndex());
    
    mShader->BindBuffer(*mPlanesBuffer, "gPlaneBuffer",
                        commandBuffer->CurrentBufferIndex());

    mShader->BindBuffer(*mMaterialsBuffer, "gMaterialBuffer",
        commandBuffer->CurrentBufferIndex());

    mShader->BindBuffer(*mAABBsBuffer, "gAABBBuffer",
        commandBuffer->CurrentBufferIndex());

    mShader->BindBuffer(*mMeshes.at(0).GetVertexBuffer(), "gVertexBuffer",
        commandBuffer->CurrentBufferIndex());
    
    mShader->BindBuffer(*mMeshes.at(0).GetIndexBuffer(), "gIndexBuffer",
                        commandBuffer->CurrentBufferIndex());
    
}

bool RayTracerApp::MoveCamera(Camera& camera, float dt) {
    bool moved = false;
    glm::vec3 right = glm::normalize(glm::cross(camera.forward, up));
    if (mWindow.IsKeyDown(GLFW_KEY_W)) {
        camera.position += camera.forward * dt;
        moved = true;
    }
    if (mWindow.IsKeyDown(GLFW_KEY_S)) {
        camera.position -= camera.forward * dt;
        moved = true;
    }
    if (mWindow.IsKeyDown(GLFW_KEY_A)) {
        camera.position -= right * dt;
        moved = true;
    }
    if (mWindow.IsKeyDown(GLFW_KEY_D)) {
        camera.position += right * dt;
        moved = true;
    }
    if (mWindow.IsKeyDown(GLFW_KEY_LEFT_SHIFT)) {
        camera.position -= up * dt;
        moved = true;
    }
    if (mWindow.IsKeyDown(GLFW_KEY_SPACE)) {
        camera.position += up * dt;
        moved = true;
    }
    
    return moved;
}

void RayTracerApp::UpdateBuffers() {
    mSpheresBuffer = std::make_shared<Buffer<Sphere>>(
        mVulkanManager, mSpheres.data(), sizeof(Sphere) * mSpheres.size(),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    
    mPlanesBuffer = std::make_shared<Buffer<Plane>>(
        mVulkanManager, mPlanes.data(), sizeof(Plane) * mPlanes.size(),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    mMaterialsBuffer = std::make_shared<Buffer<Material>>(
        mVulkanManager, mMaterials.data(), sizeof(Material) * mMaterials.size(),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
}

AABB RayTracerApp::CalculateAABB(const std::shared_ptr<Mesh>& mesh) {
    AABB aabb;
    aabb.min = glm::vec3{ FLT_MAX };
    aabb.max = glm::vec3{ -FLT_MAX };

    for (const auto& vertex : mesh->GetVertices()) {
        aabb.min = glm::min(aabb.min, vertex.position);
        aabb.max = glm::max(aabb.max, vertex.position);
    }

    return aabb;
}