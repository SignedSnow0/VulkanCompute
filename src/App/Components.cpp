#include "Components.h"

#include <imgui.h>

bool EntityView::DrawSpheres(std::vector<Sphere>& spheres, std::vector<Material>& materials) {
    bool changed = false;
    uint32_t id = 0;
    for (auto& sphere : spheres) {
        ImGui::PushID(&sphere);
        changed |= ImGui::DragFloat3("Position", &sphere.position.x, 0.01f);
        changed |= ImGui::DragFloat("Radius", &sphere.radius, 0.01f);

        changed |= ImGui::ColorEdit3("Color", &materials[sphere.materialIndex].color.x, 0.01f);
        changed |= ImGui::SliderFloat("Metalness", &materials[sphere.materialIndex].metalness, 0.0f, 1.0f);
        changed |= ImGui::ColorEdit4("Emission Color", &materials[sphere.materialIndex].emission_color.x, 0.01f);

        ImGui::PopID();
        ImGui::Separator();

        id++;
    }

    return changed;
}

bool EntityView::DrawPlanes(std::vector<Plane>& planes, std::vector<Material>& materials) {
    bool changed = false;
    uint32_t id = 0;
    for (auto& plane : planes) {
        ImGui::PushID(&plane);
        changed |= ImGui::DragFloat3("Position", &plane.position.x, 0.01f);
        changed |= ImGui::DragFloat3("Normal", &plane.normal.x, 0.01f);

        changed |= ImGui::ColorEdit3("Color", &materials[plane.materialIndex].color.x, 0.01f);
        changed |= ImGui::SliderFloat("Metalness", &materials[plane.materialIndex].metalness, 0.0f, 1.0f);
        changed |= ImGui::ColorEdit4("Emission Color", &materials[plane.materialIndex].emission_color.x, 0.01f);

        ImGui::PopID();
        ImGui::Separator();

        id++;
    }

    return changed;
}

bool EntityView::DrawMeshes(std::vector<Mesh>& meshes, std::vector<Material>& materials) {
    bool changed = false;
    uint32_t id = 0;
    for (auto& mesh : meshes) {
        ImGui::PushID(&mesh);

        changed |= ImGui::ColorEdit3("Color", &materials[0].color.x, 0.01f);
        changed |= ImGui::SliderFloat("Metalness", &materials[0].metalness, 0.0f, 1.0f);
        changed |= ImGui::ColorEdit4("Emission Color", &materials[0].emission_color.x, 0.01f);

        ImGui::PopID();
        ImGui::Separator();

        id++;
    }

    return changed;
}