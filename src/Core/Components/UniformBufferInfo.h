#pragma once
#include "../Common/Common.h"

#include <glm/glm.hpp>

// MVP
struct UniformBufferInfo
{
    glm::mat4 ModelMatrix;
    glm::mat4 ViewMatrix;
    glm::mat4 ProjectionMatrix;
};
