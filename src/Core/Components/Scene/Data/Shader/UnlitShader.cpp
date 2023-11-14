#include "Core/Components/Scene/Data/Shader/UnlitShader.h"
#include "Core/VulkanEngineApplication.h"
using VKHelper = VulkanEngineApplication;

UnlitShader::UnlitShader() : ShaderBase(string(magic_enum::enum_name(GetShaderType()).data()))
{
}
UnlitShader::~UnlitShader()
{
}

#pragma region Public
#pragma endregion
#pragma region Protected
ShaderType UnlitShader::GetShaderType()
{
    return ShaderType::Unlit;
}
#pragma endregion