#pragma once
#include <cstdint>
#include "vec.hpp"

struct Ray {
    // Ray properties
    Vec3 position;
    Vec3 direction;
};

struct Material {
    Vec3 color;
    Vec3 emissionColor;
    float emissionStrength;
    float metalness;
};

struct Sphere {
    Vec3 center;
    float radius;
    Material* material;
};

struct RayHit {
    Vec3 position;
    Vec3 normal;
    float distance;
    const Material* material;
};

struct Plane {
    Vec3 position;
    Vec3 normal;
    Material* material;
};

struct Triangle {
    Vec3 v0;
    Vec3 v1;
    Vec3 v2;
};

struct Vertex {
    Vec3 position;
    Vec3 normal;
    Vec2 uv;
};

