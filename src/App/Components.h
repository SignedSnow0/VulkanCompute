#pragma once
#include <vector>
#include "App/UBOs.h"
#include "Core/AssetManager.h"
#include "Core/Scene.h"

class EntityView {
public:
    EntityView() = delete;

    static bool DrawSpheres(const std::shared_ptr<Scene>& scene);
    static bool DrawPlanes(const std::shared_ptr<Scene>& scene);
    static bool DrawMeshes(const std::shared_ptr<Scene>& scene);
};