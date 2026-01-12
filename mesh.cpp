#include "mesh.h"

#include <iostream>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "math.h"

Mesh* Mesh::FromFile(const std::string& file)  {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, file.c_str())) {
        std::cerr << "Failed to load OBJ file: " << file << std::endl;
        return nullptr;
    }

    Mesh* mesh = new Mesh();

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex = {};
            vertex.position = Vec3(
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            );
            if (index.normal_index >= 0) {
                vertex.normal = Vec3(
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                );
            }
            if (index.texcoord_index >= 0) {
                vertex.uv = Vec2(
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1]
                );
            }
            mesh->vertices.push_back(vertex);
            mesh->indices.push_back(static_cast<uint32_t>(mesh->vertices.size() - 1));
        }
    }

    mesh->material.color = Vec3(1, .64, .22);
    mesh->material.emissionColor = Vec3(0.0f, 0.0f, 0.0f);
    mesh->material.emissionStrength = 0.0f;
    mesh->material.metalness = .2;

    return mesh;
}

bool Mesh::intersect(const Ray& ray, RayHit& closestHit) const {
    closestHit.distance = 1e30f;
    bool hitSomething = false;
    for (uint32_t index = 0; index < indices.size(); ) {
        const Vertex& v0 = vertices[indices[index++]];
        const Vertex& v1 = vertices[indices[index++]];
        const Vertex& v2 = vertices[indices[index++]];

        Triangle test = { v0.position, v1.position, v2.position };
        RayHit hit;

        if (::intersect(ray, test, hit) && (hit.distance < closestHit.distance)) {
            closestHit = hit;
            closestHit.material = &material;
            hitSomething = true;
        }
    }

    return hitSomething;
}