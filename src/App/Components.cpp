#include "Components.h"

#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

void decompose(const glm::mat4& matrix, glm::vec3& scale, glm::vec3& rotation, glm::vec3& translation) {
    translation = glm::vec3(matrix[3]);

    scale.x = glm::length(glm::vec3(matrix[0]));
    scale.y = glm::length(glm::vec3(matrix[1]));
    scale.z = glm::length(glm::vec3(matrix[2]));

    glm::mat4 rotationMatrix = matrix;
    rotationMatrix[0] /= scale.x;
    rotationMatrix[1] /= scale.y;
    rotationMatrix[2] /= scale.z;

    rotation = glm::degrees(glm::eulerAngles(glm::quat_cast(rotationMatrix)));
}

bool EntityView::DrawSpheres(const std::shared_ptr<Scene>& scene) {
    bool changed = false;
    scene->VisitSphere([&](Sphere& sphere, Material& material) {
        ImGui::PushID(&sphere);
        changed |= ImGui::DragFloat3("Position", &sphere.position.x, 0.01f);
        changed |= ImGui::DragFloat("Radius", &sphere.radius, 0.01f);

        changed |= ImGui::ColorEdit3("Color", &material.color.x, 0.01f);
        changed |= ImGui::SliderFloat("Metalness", &material.metalness, 0.0f, 1.0f);
        changed |= ImGui::ColorEdit4("Emission Color", &material.emission_color.x, 0.01f);

        ImGui::PopID();
        ImGui::Separator();
        return changed;
    });

    return changed;
}

bool EntityView::DrawPlanes(const std::shared_ptr<Scene>& scene) {
    bool changed = false;
    scene->VisitPlane([&](Plane& plane, Material& material) {
        ImGui::PushID(&plane);
        changed |= ImGui::DragFloat3("Position", &plane.position.x, 0.01f);
        changed |= ImGui::DragFloat3("Normal", &plane.normal.x, 0.01f);

        changed |= ImGui::ColorEdit3("Color", &material.color.x, 0.01f);
        changed |= ImGui::SliderFloat("Metalness", &material.metalness, 0.0f, 1.0f);
        changed |= ImGui::ColorEdit4("Emission Color", &material.emission_color.x, 0.01f);

        ImGui::PopID();
        ImGui::Separator();

        return changed;
    });

    return changed;
}


bool EntityView::DrawMeshes(const std::shared_ptr<Scene>& scene) {
    bool changed = false;
    scene->VisitModel([&](Model& model, Material& material) {
        ImGui::PushID(&model);

        auto& modelMatrix = model.GetModelMatrix();
        glm::vec3 translation, rotation, scale;
        decompose(modelMatrix, scale, rotation, translation);

        changed |= ImGui::DragFloat3("Translation", &translation.x, 0.01f);
        changed |= ImGui::SliderFloat3("Rotation", &rotation.x, 0, 180);
        changed |= ImGui::DragFloat3("Scale", &scale.x, 0.01f);

        if (changed) {
            model.SetModelMatrix(glm::translate(glm::mat4(1.0f), translation) *
                          glm::toMat4(glm::quat{ glm::radians(rotation) }) *
                          glm::scale(glm::mat4(1.0f), scale));
        }

        changed |= ImGui::ColorEdit3("Color", &material.color.x, 0.01f);
        changed |= ImGui::SliderFloat("Metalness", &material.metalness, 0.0f, 1.0f);
        changed |= ImGui::ColorEdit4("Emission Color", &material.emission_color.x, 0.01f);

        ImGui::PopID();
        ImGui::Separator();

        return changed;
    });

    return changed;
}