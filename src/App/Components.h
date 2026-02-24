#pragma once
#include <vector>
#include "App/UBOs.h"
#include "Core/AssetManager.h"

class EntityView {
public:
    EntityView() = delete;

    static bool DrawSpheres(std::vector<Sphere>& spheres, std::vector<Material>& materials);
    static bool DrawPlanes(std::vector<Plane>& planes, std::vector<Material>& materials);
    static bool DrawMeshes(std::vector<Mesh>& meshes, std::vector<Material>& materials);
};