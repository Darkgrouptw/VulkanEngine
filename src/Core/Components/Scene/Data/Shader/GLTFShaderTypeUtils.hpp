#pragma once
#if USE_ASSIMP
#include "Core/Components/Scene/Data/Shader/ShaderType.h"
#include "Core/Components/Scene/Data/Shader/ShaderTypeUtils.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// 跟 Shader 相關的 Utils
class GLTFShaderTypeUtils
{
public:
	static ShaderType GetShaderTypeFrom(aiShadingMode mode)
	{
		switch (mode)
		{
		case aiShadingMode_PBR_BRDF:
			mType = ShaderType::PBR;
			break;
		default:
			mType = ShaderType::Unlit;
			break;
		}
		return mType;
	}
private:
	static inline ShaderType mType									= ShaderType::Unlit;
};
#endif