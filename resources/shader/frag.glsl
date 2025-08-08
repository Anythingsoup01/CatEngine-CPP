#version 430 core
out vec4 oFragColor;

in vec4 oColor;

void main()
{
    oFragColor = oColor;
}
