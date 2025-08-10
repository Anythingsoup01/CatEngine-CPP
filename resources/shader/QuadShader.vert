#version 430

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_ModelMatrix;

out vec4 oColor;
out vec2 oTexCoord;

void main()
{
    gl_Position = u_ViewProjection * u_ModelMatrix * vec4(aPosition, 1.0);
    oColor = vec4(aColor, 1.0f);
    oTexCoord = aTexCoord;
}
