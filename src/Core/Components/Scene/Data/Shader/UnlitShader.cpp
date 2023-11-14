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
vector<VkDescriptorSetLayoutBinding> UnlitShader::GetVKDescriptorSetLayoutBinding()
{
    return CommonSetupForGetVKDescriptorSetLayoutBinding();
}
vector<VkDeviceSize> UnlitShader::GetVKBufferSize()
{
    return CommonSetupForGetVKBufferSize();
}
vector<VkDescriptorPoolSize> UnlitShader::GetVKDescriptorSize()
{
    return CommonSetupForGetVKDescriptorSize();
}
vector<VkWriteDescriptorSet> UnlitShader::GetVKWriteDescriptorSet(size_t pFrameIndex)
{
    return CommonSetupForGetVKWriteDescriptorSet(pFrameIndex);
}
ShaderType UnlitShader::GetShaderType()
{
    return ShaderType::Unlit;
}
#pragma endregion