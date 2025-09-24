#version 460

layout (rgba16f, set = 0, binding = 0) uniform image2D framebuffer;

layout (local_size_x = 16, local_size_y = 16) in;
void main()
{
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv = (vec2(pixel_coords) + vec2(0.5)) / vec2(imageSize(framebuffer));
    vec3 color = vec3(uv, 0.5 + 0.5 * sin(uv.x * 10.0));
    imageStore(framebuffer, pixel_coords, vec4(color, 1.0));
}