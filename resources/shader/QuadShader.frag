#version 430 core

layout(location = 0) out vec4 oFragColor;

uniform sampler2D u_TextureID;

in vec4 oColor;
in vec2 oTexCoord;

void main()
{
    oFragColor = texture(u_TextureID, oTexCoord) * oColor;
}
