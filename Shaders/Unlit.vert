#version 450
#include "CommonVertex.glsl"

// In
// 可以使用 devc2 => double vec
layout (location = 0) in vec3 InPosition;
layout (location = 1) in vec3 InNormal;
layout (location = 2) in vec4 InVertexColor;
layout (location = 3) in vec2 InTexcoord;

// Uniform
layout (binding = 0) uniform UniformBufferInfo {
    mat4 ModelMatrix;
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
} UBObject;

// Out
layout (location = 0) out vec4 OutVertexColor;
layout (location = 1) out vec3 OutBaryCoordinate;

void main()
{
    gl_Position                                                     = UBObject.ProjectionMatrix * UBObject.ViewMatrix * UBObject.ModelMatrix * vec4(InPosition, 1);

    // Send to Fragment
    OutVertexColor                                                  = InVertexColor;
    OutBaryCoordinate                                               = BaryCoordPos[gl_VertexIndex % 3];
}