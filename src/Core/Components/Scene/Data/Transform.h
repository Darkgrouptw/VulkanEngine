#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <string>

using namespace std;

class Transform
{
public:
    string Name;

    glm::vec4 Position;
    glm::quat Rotation;
    glm::vec3 Scale;
};