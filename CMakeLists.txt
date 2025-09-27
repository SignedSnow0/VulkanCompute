cmake_minimum_required(VERSION 3.16)
project(VulkanCompute)

add_executable(VulkanCompute
    src/main.cpp
    src/VulkanManager.cpp
    src/Window.cpp
    src/Surface.cpp
    src/Shader.cpp
    src/Pipeline.cpp
    src/RenderPass.cpp
    src/CommandBuffer.cpp
    src/Image.cpp
    src/Buffer.cpp
)

set_property(TARGET VulkanCompute PROPERTY CXX_STANDARD 20)

target_link_directories(VulkanCompute PRIVATE "$ENV{VULKAN_SDK}/lib/")
target_link_libraries(VulkanCompute PRIVATE vulkan-1.lib shaderc_combinedd.lib spirv-cross-cored.lib glfw)

target_include_directories(VulkanCompute PRIVATE "$ENV{VULKAN_SDK}/include/" external/glfw/include, external/stb)

add_subdirectory(external/glfw)
