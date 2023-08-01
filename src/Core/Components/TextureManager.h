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
    TextureManager(string,
        function<void(VkDeviceSize, VkBuffer&, VkDeviceMemory&)>,
        VkDevice&,
        function<uint32_t(uint32_t, VkMemoryPropertyFlags)>,
        function<VkCommandBuffer()>,
        function<void(VkCommandBuffer)>,
        VkFormat);
    ~TextureManager();

    void CreateImageView();                                                                                 // 建立將對應的 Texture 建立 TextureImage View
    void CreateSampler(VkPhysicalDevice);                                                                   // 建立對應的 Sampler
private:
    void CreateImage(int, int, function<uint32_t(uint32_t, VkMemoryPropertyFlags)>);                        // 建立 Vulkan 的 Image & buffer
    void TransitionImageLayout(VkImageLayout, VkImageLayout);                                               // 建立 VkCommandBuffer
    void CopyBufferToImage(VkBuffer, uint32_t, uint32_t);                                                   // 建立 Buffer 到 Image 中

    void __GenerateImageSubResourceRange(VkImageSubresourceRange&);                                         // 填寫 ImageSubResource

    VkImage mImage;                                                                                         // 貼圖
    VkDeviceMemory mImageMemory;                                                                            // 貼圖的 Memory
    VkImageView mImageView;                                                                                 // Image View
    VkSampler mImageSampler;                                                                                // Texture Image Sampler
    VkDevice mDevice;                                                                                       // 暫存裝置，刪除用
    VkFormat mFormat;                                                                                       // 圖片格式


    // Command Single Time Buffer Function
    function<VkCommandBuffer()> mBeginBufferFunc                    = nullptr;
    function<void(VkCommandBuffer)> mEndBufferFunc                  = nullptr;
};