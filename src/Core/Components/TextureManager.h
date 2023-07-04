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
    TextureManager(string, function<void(VkDeviceSize, VkBuffer&, VkDeviceMemory&)>);
    ~TextureManager();

    //void* LoadImage();
    //void ReleaseImage();
    //void CreateImage();


    //VkDeviceSize GetTextureSize();
private:
    stbi_uc* LoadImageToRAM(string);                                                                        // 讀圖到 RAM 中
    void UploadImageToVRAM(stbi_uc*, VkBuffer&, VkDeviceMemory&);                                           // 上傳到 GPU

	//void LoadImageAndUploadToGPU(string);                                                                   // 讀圖 & 並上傳到 GPU 

    stbi_uc* mPixels;                                                                                       // 像素資料
    int mWidth, mHeight, mChannels;                                                                         // 圖片資訊



    //string mTexturePath;                                                                                    // 檔案路徑
    VkImage mImage;                                                                                         // 貼圖
    VkDeviceMemory mImageMemory;                                                                            // 貼圖的 Memory
};