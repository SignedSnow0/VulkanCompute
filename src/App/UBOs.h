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
    glm::mat4 viewMatrix;
    glm::vec3 position;
    glm::vec3 forward;
};