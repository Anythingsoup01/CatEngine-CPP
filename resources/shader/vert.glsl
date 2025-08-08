#version 430

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;

out vec4 oColor;

void main()
{
    gl_Position = vec4(aPosition.x, aPosition.y, aPosition.z, 1.0);
    oColor = vec4(aColor, 1.0f);
}
