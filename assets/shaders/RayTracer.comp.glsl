#version 460

struct Ray {
    vec3 origin;
    vec3 direction;
};

struct Sphere {
    vec3 center;
    float radius;
    vec3 color;
};

layout (rgba8, set = 0, binding = 0) uniform image2D framebuffer;
layout (set = 0, binding = 1) uniform Camera {
    vec3 position;
    vec3 forward;
    vec3 up;
} camera;

bool intersectSphere(Ray ray, Sphere sphere, out float t) {
    vec3 oc = ray.origin - sphere.center;
    float a = dot(ray.direction, ray.direction);
    float b = 2.0 * dot(oc, ray.direction);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    float discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0.0) {
        t = -1.0;
        return false;
    } else {
        t = (-b - sqrt(discriminant)) / (2.0 * a);
        return true;
    }
}

layout (local_size_x = 8, local_size_y = 8) in;
void main()
{
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 imageSize = imageSize(framebuffer);

    if (pixelCoords.x >= imageSize.x || pixelCoords.y >= imageSize.y) {
        return;
    }

    vec2 uv = (vec2(pixelCoords) + vec2(0.5)) / vec2(imageSize);
    uv = uv * 2.0 - 1.0;
    uv.x *= float(imageSize.x) / float(imageSize.y);

    Ray ray;
    ray.origin = camera.position;
    ray.direction = normalize(vec3(uv, -1.0));

    Sphere sphere;
    sphere.center = vec3(0.0, 0.0, -3.0);
    sphere.radius = 1.0;
    sphere.color = vec3(1.0, 0.0, 0.0);

    float t;
    vec3 pixelColor = vec3(0.5, 0.7, 1.0); // Background color

    if (intersectSphere(ray, sphere, t) && t > 0.0) {
        pixelColor = sphere.color;
    }

    imageStore(framebuffer, pixelCoords, vec4(pixelColor, 1.0));
}