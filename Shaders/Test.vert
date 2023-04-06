#version 450

// 可以使用 devc2 => double vec
layout (location = 0) in vec2 InPosition;
layout (location = 1) in vec3 InColor;

layout (location = 0) out vec3 FragColor;


void main()
{
    gl_Position                                                     = vec4(InPosition, 0, 1);
    FragColor                                                       = InColor; 
}