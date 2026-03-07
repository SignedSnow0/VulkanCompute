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
    mScene = std::make_shared<Scene>(mVulkanManager, mShader, this);
}

RayTracerApp::~RayTracerApp() = default;

void RayTracerApp::OnStart() {
    Material meshMaterial;
    meshMaterial.color = { 1, .64, .22 };
    meshMaterial.emission_color = { 0, 0, 0, 0};
    meshMaterial.metalness = .4;

    glm::vec3 translation = glm::vec3(.5, .3, 0);
    glm::vec3 rotation = glm::vec3(0, 90, 0);
    glm::vec3 scale = glm::vec3(1);

    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), translation) *
                            glm::toMat4(glm::quat{ glm::radians(rotation) }) *
                            glm::scale(glm::mat4(1.0f), scale);
    
    mScene->AddModel(Model("assets/models/Dragon_80K.obj", meshMaterial, modelMatrix));

    translation = glm::vec3(-.5, 0, 0);
    rotation = glm::vec3(90, 0, 0);
    scale = glm::vec3(3.5);

    modelMatrix = glm::translate(glm::mat4(1.0f), translation) *
                  glm::toMat4(glm::quat{ glm::radians(rotation) }) *
                  glm::scale(glm::mat4(1.0f), scale);

    mScene->AddModel(Model("assets/models/bunny.obj", meshMaterial, modelMatrix));

    BuildScene();
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
    mShader->BindImage(*mRendererImage, "window",
        commandBuffer->CurrentBufferIndex());
    
    mShader->BindUniformBuffer(
        *mSceneDataBuffer, "sceneData",
        commandBuffer
            ->CurrentBufferIndex());

    mShader->BindUniformBuffer(
        *mCamera, "camera", commandBuffer->CurrentBufferIndex());

    mScene->Draw(commandBuffer);

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
            if (EntityView::DrawSpheres(mScene)) {
                mSceneData.numFrames = 0;
            }
            ImGui::TreePop();
        }


        if (ImGui::TreeNode("Planes")) {
            if (EntityView::DrawPlanes(mScene)) {
                mSceneData.numFrames = 0;
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Meshes")) {
            if (EntityView::DrawMeshes(mScene)) {
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

    Material material;
    material.color = { .89, .85, .79 };
    material.emission_color = { 0, 0, 0, 0 };
    material.metalness = 0;
    mScene->AddPlane(boxPlane, material);

    boxPlane.position = { -1, 0, 0 };
    boxPlane.normal = { 1, 0, 0 };

    material.color = { 1, 0, 0 };
    material.emission_color = { 0, 0, 0, 0 };
    material.metalness = 0;
    mScene->AddPlane(boxPlane, material);

    boxPlane.position = { 1, 0, 0 };
    boxPlane.normal = { -1, 0, 0 };

    material.color = { 0, 1, 0 };
    material.emission_color = { 0, 0, 0, 0 };
    material.metalness = 0;
    mScene->AddPlane(boxPlane, material);

    boxPlane.position = { 0, 0, -1 };
    boxPlane.normal = { 0, 0, 1 };

    material.color = { .89, .85, .79 };
    material.emission_color = { 0, 0, 0, 0 };
    material.metalness = 0;
    mScene->AddPlane(boxPlane, material);

    boxPlane.position = { 0, 0, 1 };
    boxPlane.normal = { 0, 0, -1 };

    material.color = { .89, .85, .79 };
    material.emission_color = { 0, 0, 0, 0 };
    material.metalness = 0;
    mScene->AddPlane(boxPlane, material);

    boxPlane.position = { 0, 2, 0 };
    boxPlane.normal = { 0, -1, 0 };

    material.color = { .89, .85, .79 };
    material.emission_color = { 0, 0, 0, 0 };
    material.metalness = 0;
    mScene->AddPlane(boxPlane, material);

    Sphere sphere;
    sphere.position = { 0, 2, 0 };
    sphere.radius = .5;

    Material sphereMaterial;
    sphereMaterial.color = { 1, 1, 1 };
    sphereMaterial.emission_color = { 1, 1, 1, 1 };
    sphereMaterial.metalness = 0;
    mScene->AddSphere(sphere, sphereMaterial);
}