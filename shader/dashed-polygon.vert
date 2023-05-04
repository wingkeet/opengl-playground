#version 460 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in float inDist;

out float dist;

uniform mat4 u_mvp;

void main()
{
    dist        = inDist;
    gl_Position = u_mvp * vec4(inPos, 1.0);
}
