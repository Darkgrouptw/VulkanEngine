#pragma once
#include "../Common/Common.h"

// MVP
// alignas 是一種對齊方式
// 由於 Vulkan 會預期一些對齊方式 （例如 整個 class 要被十六整除）
// https://vulkan-tutorial.com/Uniform_buffers/Descriptor_pool_and_sets#page_Alignment-requirements
struct UniformBufferInfo
{
    alignas(16) glm::mat4 ModelMatrix;
    alignas(16) glm::mat4 ViewMatrix;
    alignas(16) glm::mat4 ProjectionMatrix;
};