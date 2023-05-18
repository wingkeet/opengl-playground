#version 460 core

layout (location = 0) in vec2 vertex_position;

layout (location = 0) uniform mat4 u_view_matrix;
layout (location = 1) uniform mat4 u_proj_matrix;
layout (location = 2) uniform float u_tf;
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

// Returns a scale matrix
mat4 scale(float sx, float sy, float sz)
{
    mat4 smat = mat4(
        sx,  0.0, 0.0, 0.0,
        0.0, sy,  0.0, 0.0,
        0.0, 0.0, sz,  0.0,
        0.0, 0.0, 0.0, 1.0);
    return smat;
}

void main()
{
    float tx = (gl_InstanceID / 6) * 0.2 - 0.9;
    float ty = (gl_InstanceID % 6) * 0.2 - 0.5;
    float tz = 0.0;
    mat4 tmat = translate(tx, ty, tz);

    float tf = 0.09 * abs(sin(u_tf * 1.0)) + 0.01; // [0.01..0.1]
    mat4 smat = scale(tf, tf, 1.0);

    mat4 model_matrix = tmat * smat;
    mat4 mv_matrix = u_view_matrix * model_matrix;

    gl_Position = u_proj_matrix * mv_matrix * vec4(vertex_position, 0.0, 1.0);

    // Rotate colors left or right or none
    const int rotate_dir = 1; // negative or positive or zero
    const int column = gl_InstanceID / 6; // [0..9]
    const int tfi = int(u_tf * 1.0) % 10; // [0..9]
    int color_index; // [0..9]
    if (rotate_dir < 0) {
        color_index = (column + tfi) % 10;
    }
    else if (rotate_dir > 0) {
        color_index = (10 + column - tfi) % 10;
    }
    else {
        color_index = column;
    }
    varying_color = u_colors[color_index];
}
