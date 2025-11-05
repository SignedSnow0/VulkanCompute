#pragma once

struct Camera {
    alignas(16) float position[3];
    alignas(16) float forward[3];
    float up[3];
};

enum class DebugView {
    None = 0,
    Normals = 1,
    Depth = 2,
};

struct Settings {
    DebugView debugView;
    float time;
};