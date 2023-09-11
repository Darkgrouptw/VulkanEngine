// 共用的 Define
#ifndef COMMON_H
#define COMMON_H

// 共用的 Include
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// GLM
#define GLM_FORCE_RADIANS                                                                                   // 角度的部分都使用 Radians 做運算
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <string>
#include <filesystem>

using namespace std;

class Common
{
public:
    static string GetResourcePath(string);
};
#endif

