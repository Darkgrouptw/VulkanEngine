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
	function<void(VkCommandBuffer)> pEndBufferFunc, VkFormat pFormat)
{
	#pragma region Use library to load image
	// 這裡要記得是圖片的資訊，和讀取出來的參數可能不一樣
	// 例；圖片可能沒有 Alpha，但是 stbi_image 讀 Alpha，拿這邊會 channel 會是 3，但資料可能會是 4
	int width, height, channels;                                                                         // 圖片資訊 

	path															= Common::GetResourcePath(path);
	stbi_uc* pixels													= stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	VkDeviceSize imageSize											= width * height * 4; // 因為適用 RGB_Alpha

	// 裝 Func
	mBeginBufferFunc												= pBeginBufferFunc;
	mEndBufferFunc													= pEndBufferFunc;

	mDevice 														= pDevice;
	mFormat															= pFormat;
	#pragma endregion
	#pragma region GPU 設定
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

	#pragma region Texture Images
	// CreateImage
	CreateImage(width, height, pFindMemoryTypeFunciton);

	// 這裡主要做幾件事
	// 1. Transition image 到 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	// 2. Copy Buffer 到 Image
	// 3. Transition image 給 Shader_Read_Only
	TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	CopyBufferToImage(stageBuffer, width, height);
	TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	// Destroy Buffer
	vkDestroyBuffer(pDevice, stageBuffer, nullptr);
	vkFreeMemory(pDevice, stageBufferMemory, nullptr);
	#pragma endregion
	#pragma endregion
}
TextureManager::~TextureManager()
{
	// Destroy Texture
	vkDestroyImageView(mDevice, mImageView, nullptr);
	vkDestroyImage(mDevice, mImage, nullptr);
	vkFreeMemory(mDevice, mImageMemory, nullptr);
}

void TextureManager::CreateImageView()
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType													= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image													= mImage;
	viewInfo.viewType												= VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format													= mFormat;
	__GenerateImageSubResourceRange(viewInfo.subresourceRange);

	if (vkCreateImageView(mDevice, &viewInfo, nullptr, &mImageView) != VK_SUCCESS)
		throw runtime_error("Failed to create texture image view");
}
void TextureManager::CreateSampler(VkPhysicalDevice pPhysicalDevice)
{
	// 抓取裝置的參數
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(pPhysicalDevice, &properties);
	float maxSamplerAnisotropy										= properties.limits.maxSamplerAnisotropy;
	cout << "Max Sampler Anisotropy: " << maxSamplerAnisotropy << endl;

	#pragma region Sampler Create Info
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType												= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	
	// 貼圖的兩種內插方式
	// 1. Oversampling (magnification filter): 當貼圖太小，被放大的時候 (有很多個 pixel 要取同一個 texel) 內插方式 (https://vulkan-tutorial.com/images/texture_filtering.png)
	// 2. Undersampling (minification filter): 當貼圖太大，但縮小的時候 (一個 pixel 要取很多個 texel) 內插方式 (https://vulkan-tutorial.com/images/anisotropic_filtering.png)
	samplerInfo.magFilter											= VK_FILTER_LINEAR;						// 1.
	samplerInfo.minFilter											= VK_FILTER_LINEAR;						// 2.

	// 超過時要怎麼拿 (https://vulkan-tutorial.com/images/texture_addressing.png)
	samplerInfo.addressModeU										= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV										= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW										= VK_SAMPLER_ADDRESS_MODE_REPEAT;

	// Anisotropic
	samplerInfo.anisotropyEnable									= VK_TRUE;
	samplerInfo.maxAnisotropy										= maxSamplerAnisotropy;					// 這個數值越大，代表越花效能，但品質會越好
	samplerInfo.borderColor											= VK_BORDER_COLOR_INT_OPAQUE_BLACK;		// 超越的時候 Black
	#pragma endregion

}

#pragma endregion
#pragma region Private
void TextureManager::CreateImage(int pWidth, int pHeight, function<uint32_t(uint32_t, VkMemoryPropertyFlags)> pFindMemoryTypeFunciton)
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

	createInfo.format												= mFormat;
	createInfo.tiling												= VK_IMAGE_TILING_OPTIMAL;				// ToDo: Check vs Linear(row-major)
	createInfo.initialLayout										= VK_IMAGE_LAYOUT_UNDEFINED;			// VK_IMAGE_LAYOUT_UNDEFINED (在初始化的時候會砍調沒有在 GPU 用掉像素資料)，VK_IMAGE_LAYOUT_PREINITIALIZED (相反，會保留) (https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImageLayout.html)
	
	createInfo.usage												= VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT; // VK_IMAGE_USAGE_SAMPLED_BIT 是設定要 VkImageCreateInfo 時設定
	createInfo.sharingMode											= VK_SHARING_MODE_EXCLUSIVE;
	createInfo.samples												= VK_SAMPLE_COUNT_1_BIT;
	createInfo.flags												= 0;

	if (vkCreateImage(mDevice, &createInfo, nullptr, &mImage) 		!= VK_SUCCESS)
		throw runtime_error("Failed to create vkimage");
	#pragma endregion
	#pragma region Allocate Memory
	VkMemoryRequirements requirement{};
	vkGetImageMemoryRequirements(mDevice, mImage, &requirement);
	
	VkMemoryAllocateInfo allocateInfo{};
	allocateInfo.sType												= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize										= requirement.size;
	allocateInfo.memoryTypeIndex									= pFindMemoryTypeFunciton(requirement.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(mDevice, &allocateInfo, nullptr, &mImageMemory) != VK_SUCCESS)
		throw runtime_error("Failed to allocate memory in image");
	vkBindImageMemory(mDevice, mImage, mImageMemory, 0);
	#pragma endregion
}
void TextureManager::TransitionImageLayout(VkImageLayout pOldLayout, VkImageLayout pNewLayout)
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
	barrier.image													= mImage;
	__GenerateImageSubResourceRange(barrier.subresourceRange);
	#pragma endregion
	#pragma region Sync
	// 這裡需要根據 type 來決定怎麼處理
	// 有兩種狀況
	// 1. new layout 是 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL 		=> 不需要等什麼
	// 2. 
	VkPipelineStageFlags sourceStage, destinationStage;
	if (pOldLayout == VK_IMAGE_LAYOUT_UNDEFINED 					&& pNewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask										= 0;
		barrier.dstAccessMask										= VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage													= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage											= VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (pOldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL		&& pNewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask										= VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask										= VK_ACCESS_SHADER_READ_BIT;

		sourceStage													= VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage											= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
		throw invalid_argument("Unsupported layout transition");

	// synchronization
	vkCmdPipelineBarrier(commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier);
	#pragma endregion
	mEndBufferFunc(commandBuffer);
}
void TextureManager::CopyBufferToImage(VkBuffer pBuffer, uint32_t pWidth, uint32_t pHeight)
{
	VkCommandBuffer commandBuffer 									= mBeginBufferFunc();
	#pragma region Buffer To Image
	VkBufferImageCopy region{};

	// 算多張圖中間 offset
	region.bufferOffset												= 0;
	region.bufferRowLength											= 0;
	region.bufferImageHeight										= 0;

	region.imageSubresource.aspectMask								= VK_IMAGE_ASPECT_COLOR_BIT;			// color, depth or stencil
	region.imageSubresource.mipLevel								= 0;
	region.imageSubresource.baseArrayLayer							= 0;
	region.imageSubresource.layerCount								= 1;
	region.imageOffset												= VkOffset3D{0, 0, 0};
	region.imageExtent												= VkExtent3D{
		pWidth,
		pHeight,
		1
	};
	#pragma endregion
	// Copy !!
	vkCmdCopyBufferToImage(
		commandBuffer,
		pBuffer,
		mImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);
	mEndBufferFunc(commandBuffer);
}

void TextureManager::__GenerateImageSubResourceRange(VkImageSubresourceRange& range)
{
	range.aspectMask												= VK_IMAGE_ASPECT_COLOR_BIT;
	range.baseMipLevel												= 0;
	range.levelCount												= 1;
	range.baseArrayLayer											= 0;
	range.layerCount												= 1;
}
#pragma endregion
