#include "Core/Components/TextureManager.h"

// 使用 STB_IMAGE_IMPLEMENTATION 來順便 Compile 實作
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#pragma region Public
TextureManager::TextureManager(string path,
	function<void(VkDeviceSize, VkBuffer&, VkDeviceMemory&)> pCreateData,
	VkDevice& pDevice,
	function<uint32_t(uint32_t, VkMemoryPropertyFlags)> pFindMemoryTypeFunciton,
	function<VkCommandBuffer()> pBeginBufferFunc,
	function<void(VkCommandBuffer)> pEndBufferFunc)
{
	// 這裡要記得是圖片的資訊，和讀取出來的參數可能不一樣
	// 例；圖片可能沒有 Alpha，但是 stbi_image 讀 Alpha，拿這邊會 channel 會是 3，但資料可能會是 4
	int width, height, channels;                                                                         // 圖片資訊 

	path															= Common::GetResourcePath(path);
	stbi_uc* pixels													= stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	VkDeviceSize imageSize											= width * height * 4; // 因為適用 RGB_Alpha

	// 裝 Func
	mBeginBufferFunc												= pBeginBufferFunc;
	mEndBufferFunc													= pEndBufferFunc;

	// 裝進 StageBuffer 中
	VkBuffer stageBuffer;
	VkDeviceMemory stageBufferMemory;
	pCreateData(imageSize, stageBuffer, stageBufferMemory);

	// Upload Data to Memory
	void* data;
	vkMapMemory(pDevice, stageBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, imageSize);
	vkUnmapMemory(pDevice, stageBufferMemory);

	// Release CPU Data
	stbi_image_free(pixels);

	// CreateImage
	CreateImage(width, height, pDevice, pFindMemoryTypeFunciton);

	// Release VKData
	vkDestroyBuffer(pDevice, stageBuffer, nullptr);
	vkFreeMemory(pDevice, stageBufferMemory, nullptr);
}
TextureManager::~TextureManager()
{
}

#pragma endregion
#pragma region Private
void TextureManager::CreateImage(int pWidth, int pHeight, VkDevice& pDevice, function<uint32_t(uint32_t, VkMemoryPropertyFlags)> pFindMemoryTypeFunciton)
{
	#pragma region Image Create
	VkImageCreateInfo createInfo{};
	createInfo.sType												= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.imageType											= VK_IMAGE_TYPE_2D;
	createInfo.extent.width											= pWidth;
	createInfo.extent.height										= pHeight;
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
void TextureManager::TransitionImageLayout(VkImage pImage, VkFormat pFormat, VkImageLayout pOldLayout, VkImageLayout pNewLayout)
{
	VkCommandBuffer commandBuffer 									= mBeginBufferFunc();

	#pragma region Barrier 設定
	VkImageMemoryBarrier barrier{};
	barrier.sType													= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout												= pOldLayout;							// 如果 oldlayout 是 VK_IMAGE_LAYOUT_UNDEFINED，代表不在乎前面的 layout 設定
	barrier.newLayout												= pNewLayout;

	// 設定 QueueIndex，設定 VK_QUEUE_FAMILY_IGNORED 代表忽略 QueueFamilyIndex
	barrier.srcQueueFamilyIndex										= VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex										= VK_QUEUE_FAMILY_IGNORED;

	// 參數
	barrier.image													= pImage;
	barrier.subresourceRange.aspectMask								= VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel							= 0;
	barrier.subresourceRange.levelCount								= 1;
	barrier.subresourceRange.baseArrayLayer							= 0;
	barrier.subresourceRange.layerCount								= 1;
	#pragma endregion
	#pragma region 同步
	// synchronization
	vkCmdPipelineBarrier(commandBuffer,
		0, 0,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier);
	#pragma endregion
	mEndBufferFunc(commandBuffer);
}
void TextureManager::CopyBufferToImage()
{
	VkCommandBuffer commandBuffer 									= mBeginBufferFunc();
	mEndBufferFunc(commandBuffer);
}
#pragma endregion
