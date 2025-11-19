#include "RayTracerApp.h"
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
}

RayTracerApp::~RayTracerApp() = default;

void RayTracerApp::OnStart() {}

void RayTracerApp::OnUpdate(float dt) {
    static RandomSeed seed;
    static SceneData scene_data{0}; //inizializzo il valore del frame a 0
    //srand(time(NULL));
    //seed.seed = rand();

    seed.seed = mRandomDistribution(mRandomGenerator);
    mSeed->UpdateData(seed); // carico i dati che mi sono creata sulla gpu
                             // attraverso il buffer
    scene_data.numFrames++; //aggiungo un frame
    mSceneData->UpdateData(scene_data);
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

    mPipeline->Dispatch(
        commandBuffer, (mSurface->Extent().width + 7) / 8,
        (mSurface->Extent().height + 7) / 8,
        1); // chiamo il kernel e definisco le dimensioni dei blocchi
}

void RayTracerApp::OnStop() {}