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
layout (binding = 1) vec3 LightPos;


// Out
layout (location = 0) out vec3 OutWorldSpaceNormal;
layout (location = 1) out vec4 OutVertexColor;
layout (location = 2) out vec3 ToLightVector;

void main()
{
    vec4 worldPos                                                   = MVPInfo.ModelMatrix * vec4(InPosition, 1);
    gl_Position                                                     = MVPInfo.ProjectionMatrix * MVPInfo.ViewMatrix * worldPos;

    // Send to Fragment
    OutWorldSpaceNormal                                             = normalize((MVPInfo.ModelMatrix * vec4(InNormal, 1)).xyz);
    OutVertexColor                                                  = InVertexColor;
    ToLightVector                                                   = normalize(LightPos - worldPos);
}