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