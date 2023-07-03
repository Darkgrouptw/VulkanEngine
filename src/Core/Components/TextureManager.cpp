#include "Core/Components/TextureManager.h"

// 使用 STB_IMAGE_IMPLEMENTATION 來順便 Compile 實作
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#pragma region Public
TextureManager::TextureManager(string path)
{
	mTexturePath = Common::GetResourcePath(path);
}
TextureManager::~TextureManager()
{
}

void* TextureManager::LoadImage()
{
	// 貼圖參數
	mPixels 														= stbi_load(mTexturePath.c_str(), &mWidth, &mHeight, &mChannels, STBI_rgb_alpha);
	return mPixels;
}
void TextureManager::ReleaseImage()
{
	// 刪除貼圖
	if (mPixels != nullptr)
		stbi_image_free(mPixels);
}

VkDeviceSize TextureManager::GetTextureSize()
{
	// 因為 STBI_rgb_alpha
	return mWidth * mHeight * 4;
}
#pragma endregion
#pragma region Private
#pragma endregion
