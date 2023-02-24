#version 450

layout (location = 0) out vec3 FragColor;

// 左上角為 (0, 0)
vec2 Positions[3] = vec2[] 
(
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);
vec3 Colors[3] = vec3[] 
(
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main()
{
    gl_Position = vec4(Positions[gl_VertexIndex], 0.0, 1);
    FragColor = Colors[gl_VertexIndex];
}