#version 460 core

layout (location = 0) in vec3 vertex_position;

layout (location = 0) uniform vec3 color;

out vec3 varying_color; // interpolated by rasterizer

void main()
{
    gl_Position = vec4(vertex_position, 1.0);
    varying_color = color;
}
