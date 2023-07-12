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
    TextureManager(string, function<void(VkDeviceSize, VkBuffer&, VkDeviceMemory&)>, VkDevice&);
    ~TextureManager();

    void UploadImageToVRAM(VkDevice&, function<uint32_t(uint32_t, VkMemoryPropertyFlags)>);                 // 上傳到 GPU
    void ReleaseCPUData();                                                                                  // 砍掉 CPU Data
private:
    stbi_uc* LoadImageToRAM(string);                                                                        // 讀圖到 RAM 中

    // 這裡要記得是圖片的資訊，和讀取出來的參數可能不一樣
    // 例；圖片可能沒有 Alpha，但是 stbi_image 讀 Alpha，拿這邊會 channel 會是 3，但資料可能會是 4
    stbi_uc* mPixels;                                                                                       // 像素資料
    int mWidth, mHeight, mChannels;                                                                         // 圖片資訊 

    VkImage mImage;                                                                                         // 貼圖
    VkDeviceMemory mImageMemory;                                                                            // 貼圖的 Memory
};