#pragma once
#include <cmath>

#include "common.h"

bool intersect(const Ray& ray, const Sphere& sphere, RayHit& hit);
bool intersect(const Ray& ray, const Plane& plane, RayHit& hit);
bool intersect(const Ray& ray, const Triangle& triangle, RayHit& hit);