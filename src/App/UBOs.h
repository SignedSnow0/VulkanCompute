#pragma once
#include <cstdint>
#include <glm/glm.hpp>

struct SceneData {
    uint32_t numFrames;
    uint32_t seed;
    uint32_t maxBounces;
    uint32_t maxBvhDepth;
};

struct Camera {
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 forward;
};

struct Material {
    glm::vec3 color;
    float metalness;
    glm::vec4 emission_color;
};

struct Sphere {
    alignas(16) glm::vec3 position;
    float radius;
    uint32_t materialIndex;
};

struct Plane {
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 normal;
    uint32_t materialIndex;
};
