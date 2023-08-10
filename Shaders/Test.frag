#version 450

layout (location = 0) in vec3 FragColor;
layout (location = 1) in vec2 FragTexcoord;

layout (binding = 1) uniform sampler2D Texture2D;

layout (location = 0) out vec4 OutColor;

void main()
{
    OutColor = texture(Texture2D, FragTexcoord);
}