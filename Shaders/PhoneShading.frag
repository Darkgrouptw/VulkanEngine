#version 450

// In
layout (location = 0) in vec4 FragColor;

// Uniform
layout (binding = 2) uniform MaterialBufferInfo{
    vec4 AmbientColor;
    vec4 DiffuseColor;
    vec4 SpecularColor;
} MatInfo;

// Out
layout (location = 0) out vec4 OutColor;

void main()
{
    OutColor = FragColor;
}