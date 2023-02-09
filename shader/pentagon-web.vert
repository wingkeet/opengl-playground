#version 460 core

layout (location = 0) in vec2 vertex_position;

layout (location = 0) uniform mat4 mv_matrix;
layout (location = 1) uniform mat4 proj_matrix;
layout (location = 2) uniform vec3 color;
layout (location = 3) uniform bool wireframe;

out vec3 varying_color; // interpolated by rasterizer

void main()
{
    gl_Position = proj_matrix * mv_matrix * vec4(vertex_position, 0.0, 1.0);
    varying_color = wireframe ? vec3(0.0) : color;
}
