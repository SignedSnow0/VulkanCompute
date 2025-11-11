#include "RayTracerApp.h"
#include <stdlib.h>
#include <time.h>

RayTracerApp::RayTracerApp()
    : VulkanComputeApp(1920, 1080, "Vulkan Ray Tracer") {
    mShader = std::shared_ptr<Shader>(Shader::Create(
        mVulkanManager, "assets/shaders/RayTracer.comp", ShaderStage::Compute,
        mSurface->ImageCount()));
    mPipeline = std::make_shared<ComputePipeline>(mVulkanManager, mShader);
    mSeed = std::make_shared<UniformBuffer<RandomSeed>>(mVulkanManager); //inizializzo il buffer
}

RayTracerApp::~RayTracerApp() = default;

void RayTracerApp::OnStart() {}

void RayTracerApp::OnUpdate(float dt) {
   RandomSeed seed;
   srand(time(NULL));
   seed.seed=rand();
   mSeed->UpdateData(seed); //carico i dati che mi sono creata sulla gpu attraverso il buffer
}

void RayTracerApp::OnRender(float dt,
                            std::shared_ptr<CommandBuffer> commandBuffer) {
    mShader->BindSurfaceAsImage(mSurface, "window",
                                commandBuffer->CurrentBufferIndex()); //memcopy host to device di window (passaggio dati)
    mShader->BindUniformBuffer(*mSeed, "random_seed", commandBuffer->CurrentBufferIndex()); // dico al shader che il valore della variabile random_seed lo devo prendere dal buffer indicato
    mPipeline->Dispatch(commandBuffer, (mSurface->Extent().width + 7) / 8,
                        (mSurface->Extent().height + 7) / 8, 1); //chiamo il kernel e definisco le dimensioni dei blocchi
}

void RayTracerApp::OnStop() {}