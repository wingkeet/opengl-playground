#version 460 core

in float dist;

out vec4 fragColor;

uniform vec2  u_resolution;
uniform float u_dashSize;
uniform float u_gapSize;

void main()
{
    if (fract(dist / (u_dashSize + u_gapSize)) > u_dashSize/(u_dashSize + u_gapSize))
        discard;
    fragColor = vec4(0.0, 0.8, 0.0, 1.0);
}
