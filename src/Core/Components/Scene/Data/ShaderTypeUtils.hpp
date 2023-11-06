#pragma once
#include "Core/Common/Common.h"
#include "Core/Components/Scene/Data/ShaderType.h"

#include <magic_enum.hpp>
#include <string>

using namespace std;

class ShaderTypeUtils
{
public:
	static string GetVertexShaderPath(ShaderType pType)				{ return Common::GetShaderPath(string(magic_enum::enum_name(pType).data()) + ".vert.spv"); }
	static string GetFragmentShaderPath(ShaderType pType)			{ return Common::GetShaderPath(string(magic_enum::enum_name(pType).data()) + ".frag.spv"); }
};
