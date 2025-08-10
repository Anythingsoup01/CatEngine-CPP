#version 430 core
out vec4 oFragColor;

in vec4 oColor;
in vec2 oTexCoord;

void main()
{
    oFragColor = vec4(oTexCoord, 0.0, 1.0);
}
