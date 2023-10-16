#pragma once
#if USE_ASSIMP
#include "Core/Components/Scene/Data/ShaderType.h"
#include "Core/Components/Scene/Data/ShaderTypeUtils.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// 跟 Shader 相關的 Utils
class GLTFShaderTypeUtils
{
public:
	static ShaderType GetShaderTypeFrom(aiShadingMode mode)
	{
		if (mode == aiShadingMode::aiShadingMode_Unlit)
		{
		}
		else
		{
			mType													= ShaderType::Unlit;
		}
		return mType;
	}
private:
	static inline ShaderType mType									= ShaderType::Unlit;
};
#endif