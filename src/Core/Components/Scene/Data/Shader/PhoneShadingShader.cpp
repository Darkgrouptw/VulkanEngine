#include "Core/Components/Scene/Data/Shader/PhoneShadingShader.h"
#include "Core/VulkanEngineApplication.h"
using VKHelper = VulkanEngineApplication;

PhoneShadingShader::PhoneShadingShader() : ShaderBase(string(magic_enum::enum_name(GetShaderType()).data()))
{
}
PhoneShadingShader::~PhoneShadingShader()
{
}

#pragma region Public
#pragma endregion
#pragma region Protected
ShaderType PhoneShadingShader::GetShaderType()
{
    return ShaderType::PhoneShading;
}
#pragma endregion