#pragma once
#include "Core/Common/Common.h"

// 只單 include 一個檔案，加速使用
//#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include <vulkan/vulkan.h>

using namespace std;

class TextureManager
{
public:
    TextureManager();
    ~TextureManager();
private:
};