#include <vector>
#include <fstream>
#include <chrono>
#include <iostream>

#include "math.h"
#include "mesh.h"
#include "vec.hpp"

constexpr int width = 1920;
constexpr int height = 1080;

Ray rayGen(Vec2 uv) { 
    Vec3 origin(0.0f, 1.0f, 2.0f);
    float aspectRatio = static_cast<float>(width) / static_cast<float>(height); 
    Vec2 tmp = Vec2(((uv.getX() / width) * 2.0f - 1.0f) * aspectRatio, ((uv.getY() / height) * 2.0f - 1.0f) * -1.0f);
    Vec3 direction = Vec3(tmp.getX(), tmp.getY(), -1.0f).normalize();
    return {origin, direction};
}

class Scene {
    public:
        Scene() {
            Material material;
            material.color = Vec3(.55, .89, 1);
            material.emissionColor = Vec3(0.0f, 0.0f, 0.0f);
            material.emissionStrength = 0.0f;
            material.metalness = .4;
            materials.push_back(material);

            material.color = Vec3(1, .34, .34);
            material.emissionColor = Vec3(0, 0, 0);
            material.emissionStrength = 0;
            material.metalness = .95;
            materials.push_back(material);

            material.color = Vec3(0, 0, 0);
            material.emissionColor = Vec3(1.0f, 1.0f, 1.0f);
            material.emissionStrength = 1.0f;
            material.metalness = 0;
            materials.push_back(material);

            material.color = Vec3(.45, .67, .44);
            material.emissionColor = Vec3(0.0f, 0.0f, 0.0f);
            material.emissionStrength = 0.0f;
            material.metalness = 0;
            materials.push_back(material);

            Sphere sphere = { Vec3(-1, 1, -2), 1, &materials[0] };
            spheres.push_back(sphere);

            sphere = {Vec3(1, 1, -2), 1, &materials[1]};
            spheres.push_back(sphere);

            sphere = {Vec3(-4, 6, -10), 5.0f, &materials[2]};
            spheres.push_back(sphere);
        
            Plane plane = {Vec3(0, 0, 0), Vec3(0.0f, 1.0f, 0.0f), &materials[3]};
            planes.push_back(plane);

            Mesh* mesh = Mesh::FromFile("viking_room.obj");
            if (mesh) {
                meshes.push_back(*mesh);
            }
        }

        Vec3 trace(int x, int y) {
            Vec2 uv(static_cast<float>(x), static_cast<float>(y));
            Ray ray = rayGen(uv);    
            Vec3 result = Vec3(0.0f, 0.0f, 0.0f);
            Vec3 ray_color = Vec3(1.0f, 1.0f, 1.0f);
            
            for (int i = 0; i < maxBounces; i++) {
                RayHit closestHit;
                if (this->closestHit(ray, closestHit)) {
                    ray.position = closestHit.position + closestHit.normal * 0.001f; // Offset to avoid self-intersection
                    Vec3 diffuseDirection = Vec3::randomInHemisphere(closestHit.normal);
                    Vec3 specularDirection = ray.direction.reflect(closestHit.normal);
                    ray.direction = diffuseDirection.lerp(specularDirection, closestHit.material->metalness).normalize();

                    Vec3 emittedLight = closestHit.material->emissionColor * closestHit.material->emissionStrength;
                    result = result + (ray_color * emittedLight);
                    ray_color = ray_color * (closestHit.material->color * closestHit.normal.dot(ray.direction));
                } else {
                    result = result + (ray_color * Vec3(0.5f, 0.7f, 1.0f)); // Background color
                    break;
                }
            }
            
            return result;
        }
    
    private: 
        bool closestHit(Ray ray, RayHit& outHit) {
            RayHit closestHit;
            closestHit.distance = 1e30f;
            bool didHit = false;

            for (const auto& sphere : spheres) {
                RayHit hit;
                if (intersect(ray, sphere, hit) && (hit.distance < closestHit.distance)) {
                    closestHit = hit;
                    didHit = true;
                }
            }
            for (const auto& plane : planes) {
                RayHit hit;
                if (intersect(ray, plane, hit) && (hit.distance < closestHit.distance)) {
                    closestHit = hit;
                    didHit = true;
                }
            }
            for (const auto& mesh : meshes) {
                RayHit hit;
                if (mesh.intersect(ray, hit) && (hit.distance < closestHit.distance)) {
                    closestHit = hit;
                    didHit = true;
                }
            }

            outHit = closestHit;
            return didHit;
        }

        std::vector<Sphere> spheres;
        std::vector<Material> materials;
        std::vector<Plane> planes;
        std::vector<Mesh> meshes;
        int maxBounces = 4;
};


int main() {
    Scene scene = Scene();
    std::ofstream output("output.ppm");
    output << "P3\n" << width << " " << height << "\n255\n";

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Process pixel at (x, y)
            Vec3 pixel = scene.trace(x, y);
            int r = static_cast<int>(255.99f * pixel.getX());
            int g = static_cast<int>(255.99f * pixel.getY());
            int b = static_cast<int>(255.99f * pixel.getZ());
            output << r << " " << g << " " << b << "\n";
        }
    }

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
}