#version 450

// In
layout (location = 0) in vec4 FragColor;

// Out
layout (location = 0) out vec4 OutColor;

void main()
{
    OutColor = FragColor;
}