#pragma once
#include <random>

const float PI = 3.14159265358979323846;

float randomNormalDistribution();

class Vec2 {
    public:
        Vec2() : x(0), y(0) {}
        Vec2(float x, float y) : x(x), y(y) {}

        float getX() const { return x; }
        float getY() const { return y; }

        Vec2 operator+(const Vec2& other) const {
            return Vec2(x + other.x, y + other.y);
        }

        Vec2 operator-(const Vec2& other) const {
            return Vec2(x - other.x, y - other.y);
        }

        Vec2 operator*(float scalar) const {
            return Vec2(x * scalar, y * scalar);
        }

        float dot(const Vec2& other) const {
            return x * other.x + y * other.y;
        }

        float length() const {
            return std::sqrt(x * x + y * y);
        }

        Vec2 normalize() const {
            float len = length();
            return Vec2(x / len, y / len);
        }
    private:
        float x, y;
};

class Vec3 {
    public: 
        Vec3() : x(0), y(0), z(0) {}
        Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

        float getX() const { return x; }
        float getY() const { return y; }
        float getZ() const { return z; }

        Vec3 operator+(const Vec3& other) const {
            return Vec3(x + other.x, y + other.y, z + other.z);
        }

        Vec3 operator-(const Vec3& other) const {
            return Vec3(x - other.x, y - other.y, z - other.z);
        }

        Vec3 operator*(float scalar) const {
            return Vec3(x * scalar, y * scalar, z * scalar);
        }

        Vec3 operator*(const Vec3& other) const {
            return Vec3(x * other.x, y * other.y, z * other.z);
        }

        float dot(const Vec3& other) const {
            return x * other.x + y * other.y + z * other.z;
        }

        Vec3 cross(const Vec3& other) const {
            return Vec3(
                y * other.z - z * other.y,
                z * other.x - x * other.z,
                x * other.y - y * other.x
            );
        }

        Vec3 normalize() const {
            float length = std::sqrt(x * x + y * y + z * z);
            return Vec3(x / length, y / length, z / length);
        }

        Vec3 reflect(const Vec3& normal) const {
            return *this - normal * 2.0f * this->dot(normal);
        }

        Vec3 lerp(const Vec3& other, float t) const {
            return *this * (1.0f - t) + other * t;
        }

        static Vec3 randomOnSphere() {
            return Vec3(randomNormalDistribution(), randomNormalDistribution(), randomNormalDistribution()).normalize();
        }

        static Vec3 randomInHemisphere(const Vec3& normal) {
            Vec3 inUnitSphere = randomOnSphere();
            if (inUnitSphere.dot(normal) > 0.0f) {
                return inUnitSphere;
            } else {
                return inUnitSphere * -1.0f;
            }
        }
    private:
        float x, y, z;
    
};


