#include "math.h"
#include <random>

bool intersect(const Ray& ray, const Sphere& sphere, RayHit& hit) {
    Vec3 oc = ray.position - sphere.center;
    float a = ray.direction.dot(ray.direction);
    float b = 2.0f * oc.dot(ray.direction);
    float c = oc.dot(oc) - sphere.radius * sphere.radius;
    float discriminant = b * b - 4 * a * c;

    if (discriminant >= 0) {
        hit.distance = (-b - std::sqrt(discriminant)) / (2.0f * a);
        hit.position = ray.position + ray.direction * hit.distance;
        hit.normal = (hit.position - sphere.center).normalize();
        hit.material = sphere.material;

        return hit.distance > 0;
    }

    return false;
}

bool intersect(const Ray& ray, const Plane& plane, RayHit& hit) {
    float denom = plane.normal.dot(ray.direction);
    if (std::abs(denom) > 1e-6) {
        Vec3 p0l0 = plane.position - ray.position;
        float t  = p0l0.dot(plane.normal) / denom;
        if (t >= 0) {
            hit.distance = t;
            hit.position = ray.position + ray.direction * hit.distance;
            hit.normal = plane.normal;
            hit.material = plane.material;
            return true;
        }
    }
    return false;
}

bool intersect(const Ray& ray, const Triangle& triangle, RayHit& hit) {
    Vec3 edge1 = triangle.v1 - triangle.v0;
    Vec3 edge2 = triangle.v2 - triangle.v0;
    Vec3 h = ray.direction.cross(edge2);
    float a = edge1.dot(h);
    if (std::abs(a) < 1e-6)
        return false; // Ray is parallel to triangle

    float f = 1.0f / a;
    Vec3 s = ray.position - triangle.v0;
    float u = f * s.dot(h);
    if (u < 0.0f || u > 1.0f)
        return false;

    Vec3 q = s.cross(edge1);
    float v = f * ray.direction.dot(q);
    if (v < 0.0f || u + v > 1.0f)
        return false;

    // Compute t to find out where the intersection point is on the line
    float t = f * edge2.dot(q);
    if (t > 1e-6) { // Ray intersection
        hit.distance = t;
        hit.position = ray.position + ray.direction * hit.distance;
        hit.normal = edge1.cross(edge2).normalize();
        return true;
    }
    
    return false; // Line intersection but not a ray intersection
}

