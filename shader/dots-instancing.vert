#version 460 core

layout (location = 0) in vec2 vertex_position;

layout (location = 0) uniform mat4 u_view_matrix;
layout (location = 1) uniform mat4 u_proj_matrix;
layout (location = 2) uniform mat4 u_scale_matrix;
layout (location = 3) uniform vec3 u_colors[10];

out vec3 varying_color; // interpolated by rasterizer

// Returns a translation matrix
mat4 translate(float tx, float ty, float tz)
{
    mat4 tmat = mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        tx,  ty,  tz,  1.0);
    return tmat;
}

void main()
{
    float tx = (gl_InstanceID / 6) * 0.2 - 0.9;
    float ty = (gl_InstanceID % 6) * 0.2 - 0.5;
    float tz = 0.0;
    mat4 tmat = translate(tx, ty, tz);

    mat4 model_matrix = tmat * u_scale_matrix;
    mat4 mv_matrix = u_view_matrix * model_matrix;

    gl_Position = u_proj_matrix * mv_matrix * vec4(vertex_position, 0.0, 1.0);
    varying_color = u_colors[gl_InstanceID / 6];
}
