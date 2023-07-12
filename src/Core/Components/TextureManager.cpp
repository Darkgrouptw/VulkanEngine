#include "Core/Components/TextureManager.h"

// 使用 STB_IMAGE_IMPLEMENTATION 來順便 Compile 實作
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#pragma region Public
TextureManager::TextureManager(string path, function<void(VkDeviceSize, VkBuffer&, VkDeviceMemory&)> pCreateData, VkDevice pDevice)
{
	path															= Common::GetResourcePath(path);
	mPixels															= LoadImageToRAM(path);
	VkDeviceSize imageSize											= mWidth * mHeight * 4; // 因為適用 RGB_Alpha

	// 裝進 StageBuffer 中
	VkBuffer stageBuffer;
	VkDeviceMemory stageBufferMemory;
	pCreateData(imageSize, stageBuffer, stageBufferMemory);
}
TextureManager::~TextureManager()
{
}

void TextureManager::UploadImageToVRAM(VkDevice& pDevice, function<uint32_t(uint32_t, VkMemoryPropertyFlags)> pFindMemoryTypeFunciton)
{
	#pragma region Image Create
	VkImageCreateInfo createInfo{};
	createInfo.sType												= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.imageType											= VK_IMAGE_TYPE_2D;
	createInfo.extent.width											= mWidth;
	createInfo.extent.height										= mHeight;
	createInfo.extent.depth											= 1;									// 這裡是 1 的原因，是因為他算是高度 (Texture 3D 的高)
	createInfo.mipLevels											= 1;
	createInfo.arrayLayers											= 1;

	createInfo.format												= VK_FORMAT_R8G8B8A8_SRGB;
	createInfo.tiling												= VK_IMAGE_TILING_OPTIMAL;				// ToDo: Check vs Linear(row-major)
	createInfo.initialLayout										= VK_IMAGE_LAYOUT_UNDEFINED;			// VK_IMAGE_LAYOUT_UNDEFINED (在初始化的時候會砍調沒有在 GPU 用掉像素資料)，VK_IMAGE_LAYOUT_PREINITIALIZED (相反，會保留) (https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImageLayout.html)
	
	createInfo.usage												= VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT; // VK_IMAGE_USAGE_SAMPLED_BIT 是設定要 VkImageCreateInfo 時設定
	createInfo.sharingMode											= VK_SHARING_MODE_EXCLUSIVE;
	createInfo.samples												= VK_SAMPLE_COUNT_1_BIT;
	createInfo.flags												= 0;

	if (vkCreateImage(pDevice, &createInfo, nullptr, &mImage) 		!= VK_SUCCESS)
		throw runtime_error("Failed to create vkimage");
	#pragma endregion
	#pragma region Allocate Memory
	VkMemoryRequirements requirement{};
	vkGetImageMemoryRequirements(pDevice, mImage, &requirement);
	
	VkMemoryAllocateInfo allocateInfo{};
	allocateInfo.sType												= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize										= requirement.size;
	allocateInfo.memoryTypeIndex									= pFindMemoryTypeFunciton(requirement.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(pDevice, &allocateInfo, nullptr, &mImageMemory) != VK_SUCCESS)
		throw runtime_error("Failed to allocate memory in image");
	vkBindImageMemory(pDevice, mImage, mImageMemory, 0);
	#pragma endregion
}
void TextureManager::ReleaseCPUData()
{
	stbi_image_free(mPixels);
}
#pragma endregion
#pragma region Private
stbi_uc* TextureManager::LoadImageToRAM(string path)
{
	return stbi_load(path.c_str(), &mWidth, &mHeight, &mChannels, STBI_rgb_alpha);
}
#pragma endregion
