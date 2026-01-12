#pragma once
#include <vector>
#include <string>

#include "common.h"

class Mesh {
public:
    static Mesh* FromFile(const std::string& file);
    bool intersect(const Ray& ray, RayHit& closestHit) const;
private:
    Material material;
    
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};