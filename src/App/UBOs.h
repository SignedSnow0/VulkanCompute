#pragma once
#include <cstdint>
#include <glm/glm.hpp>

struct RandomSeed {
    uint32_t seed;
};

struct SceneData {
    glm::mat4 modelMatrix;
    uint32_t numFrames;
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
