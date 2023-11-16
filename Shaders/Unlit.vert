#version 450

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
layout (location = 0) out vec4 FragColor;

void main()
{
    gl_Position                                                     = UBObject.ProjectionMatrix * UBObject.ViewMatrix * UBObject.ModelMatrix * vec4(InPosition, 1);

    // Send to Fragment
    FragColor                                                       = vec4(1, 0, 0, 1);
}