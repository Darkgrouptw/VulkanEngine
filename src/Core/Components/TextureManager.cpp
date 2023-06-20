#include "Core/Components/TextureManager.h"

#pragma region Public
TextureManager::TextureManager(string path)
{
	#pragma region 前置作業
	// 貼圖參數
	stbi_uc* Pixels = nullptr;
	int Width, Height, Channels;

	if (!filesystem::exists(path))
		throw runtime_error("Failed to load texture" + path);
	#pragma endregion
	#pragma region 載入貼圖
	Pixels = stbi_load(path.c_str(), &Width, &Height, &Channels, STBI_rgb_alpha);
	stbi_image_free(Pixels);
	#pragma endregion
}
TextureManager::~TextureManager()
{
}
#pragma endregion
#pragma region Private
#pragma endregion
