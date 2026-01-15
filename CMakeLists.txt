cmake_minimum_required(VERSION 3.16)
project(VulkanCompute)

file(GLOB ImGuiSources
    "external/imgui/*.cpp"
    "external/imgui/backends/imgui_impl_glfw.cpp"
    "external/imgui/backends/imgui_impl_vulkan.cpp"
)

add_executable(VulkanCompute
    src/App/main.cpp
    src/App/RayTracerApp.cpp

    src/Core/AssetManager.cpp
    src/Core/Logger.cpp
    src/Core/VulkanComputeApp.cpp
    src/Core/Window.cpp

    src/Vulkan/Buffer.hpp
    src/Vulkan/CommandBuffer.cpp
    src/Vulkan/Gui.cpp
    src/Vulkan/Image.cpp
    src/Vulkan/Pipeline.cpp

    src/Vulkan/RenderPass.cpp
    src/Vulkan/Shader.cpp
    src/Vulkan/Surface.cpp
    src/Vulkan/Utils.cpp
    src/Vulkan/VulkanManager.cpp

    ${ImGuiSources}
)

set_property(TARGET VulkanCompute PROPERTY CXX_STANDARD 20)

target_link_directories(VulkanCompute PRIVATE "$ENV{VULKAN_SDK}/lib/")
target_link_libraries(VulkanCompute PRIVATE vulkan-1.lib glfw assimp)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4099")
    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /ignore:4099")

    target_link_libraries(VulkanCompute PRIVATE shaderc_sharedd.lib spirv-cross-cored.lib)
else()
    target_link_libraries(VulkanCompute PRIVATE shaderc_shared.lib spirv-cross-core.lib)
endif()

target_include_directories(VulkanCompute PRIVATE "src" "$ENV{VULKAN_SDK}/include/" "external/glfw/include" "external/stb" "external/assimp/include" "external/imgui" "external/imgui/backends")

add_subdirectory(external/glfw)
add_subdirectory(external/assimp)