#version 460 core

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_color;

layout (location = 0) uniform mat4 mv_matrix;
layout (location = 1) uniform mat4 proj_matrix;

out vec3 varying_color;

void main()
{
    gl_Position = proj_matrix * mv_matrix * vec4(vertex_position, 1.0);
    varying_color = vertex_color;
}
