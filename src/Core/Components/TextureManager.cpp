#include "Core/Components/TextureManager.h"

// 使用 STB_IMAGE_IMPLEMENTATION 來順便 Compile 實作
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#pragma region Public
TextureManager::TextureManager(string path, function<void(VkDeviceSize, VkBuffer&, VkDeviceMemory&)> createData)
{
	path															= Common::GetResourcePath(path);
	auto pixels														= LoadImageToRAM(path);
	VkDeviceSize imageSize											= mWidth * mHeight * 4; // 因為適用 RGB_Alpha

	// 裝進 StageBuffer 中
	VkBuffer stageBuffer;
	VkDeviceMemory stageBufferMemory;
	createData(imageSize, stageBuffer, stageBufferMemory);

	//UploadImageToVRAM(pixels, &stageBuffer, &stageBufferMemory)
}
TextureManager::~TextureManager()
{
}

//void* TextureManager::LoadImage()
//{
//	// 貼圖參數
//	mPixels 														= ;
//	return mPixels;
//}
//void TextureManager::ReleaseImage()
//{
//	// 刪除貼圖
//	if (mPixels != nullptr)
//		stbi_image_free(mPixels);
//}

//VkDeviceSize TextureManager::GetTextureSize()
//{
//	// 因為 STBI_rgb_alpha
//	return mWidth * mHeight * 4;
//}
#pragma endregion
#pragma region Private
stbi_uc* TextureManager::LoadImageToRAM(string path)
{
	return stbi_load(path.c_str(), &mWidth, &mHeight, &mChannels, STBI_rgb_alpha);
}
void TextureManager::UploadImageToVRAM(stbi_uc*, VkBuffer&, VkDeviceMemory&)
{

}
#pragma endregion
