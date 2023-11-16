#pragma once
#include "Core/Common/Common.h"
#include "Core/Components/Scene/Data/Shader/ShaderType.h"
#include "Core/Components/Scene/Data/Shader/UnlitShader.h"
#include "Core/Components/Scene/Data/Shader/PBRShader.h"

#include <magic_enum.hpp>
#include <string>

using namespace std;

class ShaderTypeUtils
{
public:
	static string GetVertexShaderPath(ShaderType pType)				{ return Common::GetShaderPath(string(magic_enum::enum_name(pType).data()) + ".vert.spv"); }
	static string GetFragmentShaderPath(ShaderType pType)			{ return Common::GetShaderPath(string(magic_enum::enum_name(pType).data()) + ".frag.spv"); }

	static ShaderBase* GenerateShaderFactory(ShaderType pType)
	{
		switch (pType)
		{
		case ShaderType::Unlit:
			return new UnlitShader();
			break;
		case ShaderType::PBR:
			return new PBRShader();
			break;
		default:
			break;
		}
		cout << "Generate ShaderFactory Factory failed: temp generate unlit instead" << endl;
		return new UnlitShader();
	}
};
