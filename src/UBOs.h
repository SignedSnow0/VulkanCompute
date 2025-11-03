#pragma once

struct Camera {
    alignas(16) float position[3];
    alignas(16) float forward[3];
    float up[3];
};