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

    void* LoadImage();
    void ReleaseImage();

    VkDeviceSize GetTextureSize();
    int GetWidth() { return mWidth; }
    int GetHeight() { return mHeight; }
    int GetChannels() { return mChannels; }
private:
    stbi_uc* mPixels;                                                                                       // 像素資料
    int mWidth, mHeight, mChannels;                                                                         // 圖片資訊

    string mTexturePath;                                                                                    // 檔案路徑
    VkImage mImage;                                                                                         // 貼圖
    VkDeviceMemory mImageMemory;                                                                            // 貼圖的 Memory
};