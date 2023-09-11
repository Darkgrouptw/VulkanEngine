#pragma once
#include "Core/Common/Common.h"

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