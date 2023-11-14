#pragma once
#include "Core/Components/Scene/Data/Shader/ShaderBase.h"

class UnlitShader : public ShaderBase
{
public:
	UnlitShader();
	~UnlitShader();

protected:
	vector<VkDescriptorSetLayoutBinding> GetVKDescriptorSetLayoutBinding() override;
	vector<VkDeviceSize> GetVKBufferSize() override;
	vector<VkDescriptorPoolSize> GetVKDescriptorSize() override;
	vector<VkWriteDescriptorSet> GetVKWriteDescriptorSet(size_t) override;
    ShaderType GetShaderType() override;
};