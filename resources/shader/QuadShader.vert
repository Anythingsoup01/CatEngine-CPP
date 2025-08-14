#version 430 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in float a_TilingFactor;

uniform mat4 u_ViewProjection;

layout(location = 0) out vec4 o_Color;
layout(location = 1) out vec2 o_TexCoord;
layout(location = 2) out flat float o_TexIndex;
layout(location = 3) out float o_TilingFactor;

void main()
{
    gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
    o_Color = a_Color;
    o_TexCoord = a_TexCoord;
    o_TexIndex = a_TexIndex;
    o_TilingFactor = a_TilingFactor;
}
