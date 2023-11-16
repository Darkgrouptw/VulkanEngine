#version 450
#include "Common.glsl"

// In
layout (location = 0) in vec4 VertexColor;
layout (location = 1) in vec3 BaryCoordinate;

// Out
layout (location = 0) out vec4 OutColor;


void main()
{
    OutColor = mix(BorderColor, vec4(1, 0, 0, 1), EdgeFactor(BaryCoordinate));
}