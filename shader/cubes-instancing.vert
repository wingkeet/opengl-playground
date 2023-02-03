#version 460 core

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_color;

layout (location = 0) uniform mat4 view_matrix;
layout (location = 1) uniform mat4 proj_matrix;
layout (location = 2) uniform float tf;

out vec3 varying_color;

// Returns a rotation matrix around the X axis
mat4 rotate_x(float radians)
{
    mat4 rx = mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, cos(radians), -sin(radians), 0.0,
        0.0, sin(radians), cos(radians), 0.0,
        0.0, 0.0, 0.0, 1.0);
    return rx;
}

// Returns a rotation matrix around the Y axis
mat4 rotate_y(float radians)
{
    mat4 ry = mat4(
        cos(radians), 0.0, sin(radians), 0.0,
        0.0, 1.0, 0.0, 0.0,
        -sin(radians), 0.0, cos(radians), 0.0,
        0.0, 0.0, 0.0, 1.0);
    return ry;
}

// Returns a rotation matrix around the Z axis
mat4 rotate_z(float radians)
{
    mat4 rz = mat4(
        cos(radians), -sin(radians), 0.0, 0.0,
        sin(radians), cos(radians), 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0);
    return rz;
}

// Returns a translation matrix
mat4 translate(float tx, float ty, float tz)
{
    mat4 trans = mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        tx, ty, tz, 1.0);
    return trans;
}

void main()
{
    // Value based on time factor, but different for each cube instance
    float i = gl_InstanceID + tf;

    mat4 rx = rotate_x(1.75 * i);
    mat4 ry = rotate_y(1.75 * i);
    mat4 rz = rotate_z(1.75 * i);

    float tx = sin(0.35 * i) * 8.0;
    float ty = sin(0.52 * i) * 8.0;
    float tz = sin(0.70 * i) * 8.0;

    mat4 trans = translate(tx, ty, tz);

    // Build the model matrix and then the model-view matrix
    mat4 model_matrix = trans * rx * ry * rz;
    mat4 mv_matrix = view_matrix * model_matrix;

    gl_Position = proj_matrix * mv_matrix * vec4(vertex_position, 1.0);
    varying_color = vertex_color;
}
