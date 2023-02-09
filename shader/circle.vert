#version 460 core

layout (location = 0) in vec2 vertex_position;

layout (location = 0) uniform vec3 color;

out vec3 varying_color; // interpolated by rasterizer

void main()
{
    gl_Position = vec4(vertex_position, 0.0, 1.0);
    varying_color = color;
}
