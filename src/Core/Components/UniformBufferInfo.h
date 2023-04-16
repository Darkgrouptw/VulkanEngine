#pragma once
#include "../Common/Common.h"

// MVP
struct UniformBufferInfo
{
    glm::mat4 ModelMatrix;
    glm::mat4 ViewMatrix;
    glm::mat4 ProjectionMatrix;
};