#version 450

// 可以使用 devc2 => double vec
layout (location = 0) in vec2 InPosition;
layout (location = 1) in vec3 InColor;
layout (location = 2) in vec2 InTexcoord;

layout (binding = 0) uniform UniformBufferInfo {
    mat4 ModelMatrix;
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
} UBObject;

layout (location = 0) out vec3 FragColor;
layout (location = 1) out vec2 FragTexcoord;

void main()
{
    //gl_Position                                                     = UBObject.ProjectionMatrix * UBObject.ViewMatrix * UBObject.ModelMatrix * vec4(InPosition, 0, 1);
    gl_Position                                                     = vec4(InPosition, 0, 1);

    // Send to Fragment
    FragColor                                                       = InColor;
    FragTexcoord                                                    = InTexcoord;
}