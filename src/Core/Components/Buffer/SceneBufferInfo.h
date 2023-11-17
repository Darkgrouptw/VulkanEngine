#pragma once
#include "Core/Common/Common.h"

// Scene Buffer Info
// alignas 是一種對齊方式
// 由於 Vulkan 會預期一些對齊方式 （例如 整個 class 要被十六整除）
// https://vulkan-tutorial.com/Uniform_buffers/Descriptor_pool_and_sets#page_Alignment-requirements
struct SceneBufferInfo
{
	alignas(16) glm::vec3 CameraPos;
	alignas(16) glm::vec3 LightPos;
	alignas(16) glm::vec3 LightColor;
};