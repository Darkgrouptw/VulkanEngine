#pragma once
#include "Core/Common/Common.h"

// 只單 include 一個檔案，加速使用
#include "stb/stb_image.h"

#include <vulkan/vulkan.h>
#include <filesystem>

using namespace std;

class TextureManager
{
public:
    TextureManager(string);
    ~TextureManager();
private:
    VkBuffer StagingBuffer;
    VkDeviceMemory StagingBufferMemory;
};