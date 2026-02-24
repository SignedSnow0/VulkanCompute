#include "RayTracerApp.h"

#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "App/Components.h"

static constexpr uint32_t sMaxBvhDepth = 16;
static constexpr glm::vec3 up = glm::vec3(0, 1, 0); 

RayTracerApp::RayTracerApp()
    : VulkanComputeApp(1920, 1080, "Vulkan Ray Tracer") {
    std::random_device rd;
    mRandomGenerator = std::mt19937(rd());
    mRandomDistribution = std::uniform_int_distribution<uint32_t>();
    
    mShader = std::shared_ptr<Shader>(
        Shader::Create(mVulkanManager, "assets/shaders/RayTracer.comp",
                       ShaderStage::Compute, mSurface->ImageCount()));
    mPipeline = std::make_shared<ComputePipeline>(mVulkanManager, mShader);
    mCamera = std::make_shared<UniformBuffer<Camera>>(mVulkanManager);
    mSceneDataBuffer = std::make_shared<UniformBuffer<SceneData>>(mVulkanManager);
}

RayTracerApp::~RayTracerApp() = default;

void RayTracerApp::OnStart() {
    glm::vec3 translation = glm::vec3(0, .3, 0);
    glm::vec3 rotation = glm::vec3(0, 90, 0);
    glm::vec3 scale = glm::vec3(1);

    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), translation) * glm::toMat4(glm::quat{ glm::radians(rotation) }) * glm::scale(glm::mat4(1.0f), scale);
    mScene = AssetManager::LoadScene("assets/models/Dragon_80K.obj", modelMatrix);
    for (auto& mesh : mScene->GetMeshes()) {
        BvhBuilder builder(mesh, sMaxBvhDepth);
        builder.Build();
        mBvhRenderer = std::make_unique<BvhRenderer>(mVulkanManager, builder);
    }

    Material meshMaterial;
    meshMaterial.color = { 1, .64, .22 };
    meshMaterial.emission_color = { 0, 0, 0, 0};
    meshMaterial.metalness = .4;
    mMaterials.push_back(meshMaterial);

    BuildScene();

    mSpheresBuffer = std::make_shared<Buffer<Sphere>>(
        mVulkanManager, mSpheres.data(), sizeof(Sphere) * mSpheres.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    mPlanesBuffer = std::make_shared<Buffer<Plane>>(
        mVulkanManager, mPlanes.data(), sizeof(Plane) * mPlanes.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    mMaterialsBuffer = std::make_shared<Buffer<Material>>(
        mVulkanManager, mMaterials.data(), sizeof(Material) * mMaterials.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
}

void RayTracerApp::OnUpdate(float dt) {
    static Camera camera{ glm::vec3{ 0, .5, .99 }, glm::vec3{ 0, 0, -1 } };
    mSceneData.seed = mRandomDistribution(mRandomGenerator);
    mSceneData.numFrames++;

    if (mWindow.IsKeyPressed(GLFW_KEY_R)) {
        mVulkanManager->WaitIdle();

        auto* shader =
            Shader::Create(mVulkanManager, "assets/shaders/RayTracer.comp",
                ShaderStage::Compute, mSurface->ImageCount());
        if (shader) {
            mShader = std::shared_ptr<Shader>(shader);
        }

        mPipeline = std::make_shared<ComputePipeline>(mVulkanManager, mShader);
        mSceneData.numFrames = 0;
    }

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

    if (moved) {
        mSceneData.numFrames = 0;
    }

    mSceneDataBuffer->UpdateData(mSceneData);
    mCamera->UpdateData(camera);
}

void RayTracerApp::OnRender(float dt,
                            std::shared_ptr<CommandBuffer> commandBuffer) {
    mShader->BindImage(*mRendererImage, "gWindow",
        commandBuffer->CurrentBufferIndex());
    
    mShader->BindUniformBuffer(
        *mSceneDataBuffer, "gSceneData",
        commandBuffer
            ->CurrentBufferIndex());

    mShader->BindUniformBuffer(
        *mCamera, "gCamera", commandBuffer->CurrentBufferIndex());

    mShader->BindBuffer(*mSpheresBuffer, "gSphereBuffer",
                        commandBuffer->CurrentBufferIndex());
    
    mShader->BindBuffer(*mPlanesBuffer, "gPlaneBuffer",
                        commandBuffer->CurrentBufferIndex());

    mShader->BindBuffer(*mMaterialsBuffer, "gMaterialBuffer",
        commandBuffer->CurrentBufferIndex());

    mBvhRenderer->Render(commandBuffer, mShader);

    mPipeline->Dispatch(commandBuffer, (mRendererImage->Extent().width + 7) / 8,
                        (mRendererImage->Extent().height + 7) / 8, 1);
}

void RayTracerApp::OnRenderGui(float dt) {
    RenderViewport();
    RenderSettings();
}

void RayTracerApp::OnStop() {}

void RayTracerApp::RenderViewport() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    if (ImGui::Begin("Viewport", 0, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        ImVec2 availableSize = ImGui::GetContentRegionAvail();
        ImGui::Image(mRendererImageId, availableSize);
        if (availableSize.x != static_cast<float>(mRendererImage->Extent().width) ||
            availableSize.y != static_cast<float>(mRendererImage->Extent().height)) {
            AddEndOfFrameTask([this, availableSize](const std::shared_ptr<CommandBuffer>& commandBuffer) {
                mRendererImage = std::make_shared<Image>(mVulkanManager, VkExtent2D{
                    static_cast<uint32_t>(availableSize.x),
                    static_cast<uint32_t>(availableSize.y) },
                    VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_FORMAT_R8G8B8A8_UNORM);

                mRendererImageId = mGui->RegisterImage(mRendererImage);
            });
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

void RayTracerApp::RenderSettings() {
    if (ImGui::Begin("Settings")) {
        bool changed = false;
        int id = 0;

        if (ImGui::TreeNode("Spheres")) {
            if (EntityView::DrawSpheres(mSpheres, mMaterials)) {
                AddEndOfFrameTask([this](const std::shared_ptr<CommandBuffer>& commandBuffer) {
                    mSpheresBuffer = std::make_unique<Buffer<Sphere>>(
                        mVulkanManager, mSpheres.data(), sizeof(Sphere) * mSpheres.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

                    mMaterialsBuffer = std::make_unique<Buffer<Material>>(
                        mVulkanManager, mMaterials.data(), sizeof(Material) * mMaterials.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

                    });

                mSceneData.numFrames = 0;
            }
            ImGui::TreePop();
        }


        if (ImGui::TreeNode("Planes")) {
            if (EntityView::DrawPlanes(mPlanes, mMaterials)) {
                AddEndOfFrameTask([this](const std::shared_ptr<CommandBuffer>& commandBuffer) {
                    mPlanesBuffer = std::make_unique<Buffer<Plane>>(
                        mVulkanManager, mPlanes.data(), sizeof(Plane) * mPlanes.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

                    mMaterialsBuffer = std::make_unique<Buffer<Material>>(
                        mVulkanManager, mMaterials.data(), sizeof(Material) * mMaterials.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

                    });

                mSceneData.numFrames = 0;
            }
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void RayTracerApp::BuildScene() {
    Plane boxPlane;
    boxPlane.position = { 0, 0, 0 };
    boxPlane.normal = { 0, 1, 0 };
    boxPlane.materialIndex = mMaterials.size();
    mPlanes.push_back(boxPlane);

    Material material;
    material.color = { .89, .85, .79 };
    material.emission_color = { 0, 0, 0, 0 };
    material.metalness = 0;
    mMaterials.push_back(material);

    boxPlane.position = { -1, 0, 0 };
    boxPlane.normal = { 1, 0, 0 };
    boxPlane.materialIndex = mMaterials.size();
    mPlanes.push_back(boxPlane);

    material.color = { 1, 0, 0 };
    material.emission_color = { 0, 0, 0, 0 };
    material.metalness = 0;
    mMaterials.push_back(material);

    boxPlane.position = { 1, 0, 0 };
    boxPlane.normal = { -1, 0, 0 };
    boxPlane.materialIndex = mMaterials.size();
    mPlanes.push_back(boxPlane);

    material.color = { 0, 1, 0 };
    material.emission_color = { 0, 0, 0, 0 };
    material.metalness = 0;
    mMaterials.push_back(material);

    boxPlane.position = { 0, 0, -1 };
    boxPlane.normal = { 0, 0, 1 };
    boxPlane.materialIndex = mMaterials.size();
    mPlanes.push_back(boxPlane);

    material.color = { .89, .85, .79 };
    material.emission_color = { 0, 0, 0, 0 };
    material.metalness = 0;
    mMaterials.push_back(material);

    boxPlane.position = { 0, 0, 1 };
    boxPlane.normal = { 0, 0, -1 };
    boxPlane.materialIndex = mMaterials.size();
    mPlanes.push_back(boxPlane);

    material.color = { .89, .85, .79 };
    material.emission_color = { 0, 0, 0, 0 };
    material.metalness = 0;
    mMaterials.push_back(material);

    boxPlane.position = { 0, 2, 0 };
    boxPlane.normal = { 0, -1, 0 };
    boxPlane.materialIndex = mMaterials.size();
    mPlanes.push_back(boxPlane);

    material.color = { .89, .85, .79 };
    material.emission_color = { 0, 0, 0, 0 };
    material.metalness = 0;
    mMaterials.push_back(material);

    Sphere sphere;
    sphere.position = { 0, 2, 0 };
    sphere.radius = .5;
    sphere.materialIndex = mMaterials.size();
    mSpheres.push_back(sphere);

    Material sphereMaterial;
    sphereMaterial.color = { 1, 1, 1 };
    sphereMaterial.emission_color = { 1, 1, 1, 1 };
    sphereMaterial.metalness = 0;
    mMaterials.push_back(sphereMaterial);
}