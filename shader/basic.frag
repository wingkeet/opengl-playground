#version 460 core

in vec3 varying_color;
out vec4 frag_color;

void main()
{
    frag_color = vec4(varying_color, 1.0);
}
