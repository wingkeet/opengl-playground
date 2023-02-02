#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 vertex_color;

layout (location = 0) uniform mat4 view_matrix;
layout (location = 1) uniform mat4 proj_matrix;
layout (location = 2) uniform float tf;

out vec3 color;

// Returns a matrix that performs a rotation around the X axis
mat4 buildRotateX(float rad)
{
    mat4 xrot = mat4(1.0, 0.0,      0.0,       0.0,
                     0.0, cos(rad), -sin(rad), 0.0,
                     0.0, sin(rad), cos(rad),  0.0,
                     0.0, 0.0,      0.0,       1.0);
    return xrot;
}

// Returns a matrix that performs a rotation around the Y axis
mat4 buildRotateY(float rad)
{
    mat4 yrot = mat4(cos(rad),  0.0, sin(rad), 0.0,
                     0.0,       1.0, 0.0,      0.0,
                     -sin(rad), 0.0, cos(rad), 0.0,
                     0.0,       0.0, 0.0,      1.0);
    return yrot;
}

// Returns a matrix that performs a rotation around the Z axis
mat4 buildRotateZ(float rad)
{
    mat4 zrot = mat4(cos(rad), -sin(rad), 0.0, 0.0,
                     sin(rad), cos(rad),  0.0, 0.0,
                     0.0,      0.0,       1.0, 0.0,
                     0.0,      0.0,       0.0, 1.0);
    return zrot;
}

// Returns a translation matrix
mat4 buildTranslate(float x, float y, float z)
{
    mat4 trans = mat4(1.0, 0.0, 0.0, 0.0,
                      0.0, 1.0, 0.0, 0.0,
                      0.0, 0.0, 1.0, 0.0,
                      x,   y,   z,   1.0);
    return trans;
}

void main()
{
    float i = gl_InstanceID + tf; // value based on time factor, but different for each cube instance

    float a = sin(0.35 * i) * 8.0;
    float b = sin(0.52 * i) * 8.0;
    float c = sin(0.70 * i) * 8.0;

    mat4 localRotX = buildRotateX(1.75 * i);
    mat4 localRotY = buildRotateY(1.75 * i);
    mat4 localRotZ = buildRotateZ(1.75 * i);

    mat4 localTrans = buildTranslate(a, b, c);

    // Build the model matrix and then the model-view matrix
    mat4 newM_matrix = localTrans * localRotX * localRotY * localRotZ;
    mat4 mv_matrix = view_matrix * newM_matrix;

    gl_Position = proj_matrix * mv_matrix * vec4(position, 1.0);
    color = vertex_color;
}
