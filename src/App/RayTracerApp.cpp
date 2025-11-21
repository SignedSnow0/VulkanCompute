#include "RayTracerApp.h"
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <time.h>


RayTracerApp::RayTracerApp()
    : VulkanComputeApp(1920, 1080, "Vulkan Ray Tracer") {
    std::random_device rd;
    mRandomGenerator = std::mt19937(rd());
    mRandomDistribution = std::uniform_int_distribution<uint32_t>();
    mShader = std::shared_ptr<Shader>(
        Shader::Create(mVulkanManager, "assets/shaders/RayTracer.comp",
                       ShaderStage::Compute, mSurface->ImageCount()));
    mPipeline = std::make_shared<ComputePipeline>(mVulkanManager, mShader);
    mSeed = std::make_shared<UniformBuffer<RandomSeed>>(
        mVulkanManager); // inizializzo il buffer
    mSceneData = std::make_shared<UniformBuffer<SceneData>>(
        mVulkanManager);
    mCamera = std::make_shared<UniformBuffer<Camera>>(
        mVulkanManager);

    mScene = AssetManager::LoadScene("assets/models/viking_room/viking_room.obj");
    for (auto& mesh : mScene->GetMeshes()) {
        mMeshes.push_back(MeshRenderer{ mVulkanManager, mesh });
    }
}

RayTracerApp::~RayTracerApp() = default;

void RayTracerApp::OnStart() {}

void RayTracerApp::OnUpdate(float dt) {
    static RandomSeed seed;
    static glm::mat4 modelMatrix;
    static SceneData scene_data{ 0 }; //inizializzo il valore del frame a 0
    static Camera camera{ glm::vec3{0}, glm::vec3{0,0,-1} };
    static constexpr glm::vec3 up = glm::vec3(0, 1, 0);
    glm::vec3 right = glm::normalize(glm::cross(camera.forward, up));

    if (mWindow.IsKeyPressed(GLFW_KEY_UP) || mWindow.IsKeyPressed(GLFW_KEY_W)) {
        camera.position += camera.forward * dt;
        scene_data.numFrames = 0; // resetto il numero di frame se cambio
                                 // posizione
    }
    if (mWindow.IsKeyPressed(GLFW_KEY_DOWN) || mWindow.IsKeyPressed(GLFW_KEY_S)) {
        camera.position -= camera.forward * dt;
        scene_data.numFrames = 0;
    }
    if (mWindow.IsKeyPressed(GLFW_KEY_LEFT) || mWindow.IsKeyPressed(GLFW_KEY_A)) {
        camera.position -= right * dt;
        scene_data.numFrames = 0;
    }
    if (mWindow.IsKeyPressed(GLFW_KEY_RIGHT) || mWindow.IsKeyPressed(GLFW_KEY_D)) {
        camera.position += right * dt;
        scene_data.numFrames = 0;
    }
    if (mWindow.IsKeyPressed(GLFW_KEY_LEFT_SHIFT) || mWindow.IsKeyPressed(GLFW_KEY_Q)) {
        camera.position -= up * dt;
        scene_data.numFrames = 0;
    }
    if (mWindow.IsKeyPressed(GLFW_KEY_SPACE) || mWindow.IsKeyPressed(GLFW_KEY_E)) {
        camera.position += up * dt;
        scene_data.numFrames = 0;
    }

    seed.seed = mRandomDistribution(mRandomGenerator);
    mSeed->UpdateData(seed); // carico i dati che mi sono creata sulla gpu
                             // attraverso il buffer
    modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -0.5f));
    modelMatrix =
        glm::rotate(modelMatrix, -glm::half_pi<float>(), glm::vec3(1, 0, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f));
    scene_data.numFrames++; // aggiungo un frame
    scene_data.modelMatrix = modelMatrix;
    mSceneData->UpdateData(scene_data);

    mCamera->UpdateData(camera);
}


void RayTracerApp::OnRender(float dt,
                            std::shared_ptr<CommandBuffer> commandBuffer) {
    mShader->BindSurfaceAsImage(
        mSurface, "window",
        commandBuffer->CurrentBufferIndex()); // memcopy host to device di
                                              // window (passaggio dati)
    mShader->BindUniformBuffer(
        *mSeed, "random_seed",
        commandBuffer
            ->CurrentBufferIndex()); // dico al shader che il valore della
                                     // variabile random_seed lo devo prendere
                                     // dal buffer indicato
    mShader->BindUniformBuffer(
        *mSceneData, "scene_data",
        commandBuffer
            ->CurrentBufferIndex()); // dico al shader che il valore della
                                     // variabile scene_data lo devo prendere
                                     // dal buffer indicato

    mShader->BindUniformBuffer(
        *mCamera, "camera", commandBuffer->CurrentBufferIndex());

    mShader->BindBuffer(*mMeshes.at(0).GetVertexBuffer(), "vertex_buffer",
                        commandBuffer->CurrentBufferIndex());
    mShader->BindBuffer(*mMeshes.at(0).GetIndexBuffer(), "index_buffer",
                        commandBuffer->CurrentBufferIndex());

    mPipeline->Dispatch(
        commandBuffer, (mSurface->Extent().width + 7) / 8,
        (mSurface->Extent().height + 7) / 8,
        1); // chiamo il kernel e definisco le dimensioni dei blocchi
}

void RayTracerApp::OnStop() {}