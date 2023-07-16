#pragma once
#include "Core/Common/Common.h"

// 只單 include 一個檔案，加速使用
#include "stb/stb_image.h"

#include <functional>
#include <vulkan/vulkan.h>
#include <filesystem>

using namespace std;

class TextureManager
{
public:
    TextureManager(string, function<void(VkDeviceSize, VkBuffer&, VkDeviceMemory&)>, VkDevice&, function<uint32_t(uint32_t, VkMemoryPropertyFlags)>);
    ~TextureManager();

private:
    void CreateImage(int, int, VkDevice&, function<uint32_t(uint32_t, VkMemoryPropertyFlags)>);

    VkImage mImage;                                                                                         // 貼圖
    VkDeviceMemory mImageMemory;                                                                            // 貼圖的 Memory
};