#version 450

layout (location = 0) out vec2 InPosition;
layout (location = 1) out vec3 InColor;

layout (location = 0) out vec2 FragColor;


void main()
{
    gl_Position                                                     = vec4(InPosition, 0, 1);
    FragColor                                                       = InColor; 
}