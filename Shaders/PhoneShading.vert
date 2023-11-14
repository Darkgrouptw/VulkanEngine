#version 450

// In
// 可以使用 devc2 => double vec
layout (location = 0) in vec3 InPosition;
layout (location = 1) in vec3 InNormal;
layout (location = 2) in vec4 InVertexColor;
layout (location = 3) in vec2 InTexcoord;

// Uniform
layout (binding = 0) uniform MVPBufferInfo {
    mat4 ModelMatrix;
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
} MVPInfo;
layout (binding = 1) uniform MaterialBufferInfo{
    vec4 AmbientColor;
    vec4 DiffuseColor;
    vec4 SpecularColor;
} MatInfo;


// Out
layout (location = 0) out vec4 FragColor;

void main()
{
    gl_Position                                                     = MVPInfo.ProjectionMatrix * MVPInfo.ViewMatrix * MVPInfo.ModelMatrix * vec4(InPosition, 1);

    // Send to Fragment
    FragColor                                                       = InVertexColor;
}