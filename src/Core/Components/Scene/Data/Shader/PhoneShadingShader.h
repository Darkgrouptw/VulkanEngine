#pragma once
#include "Core/Components/Scene/Data/Shader/ShaderBase.h"

class PhoneShadingShader : public ShaderBase
{
public:
	PhoneShadingShader();
	~PhoneShadingShader();

protected:
	vector<VkDescriptorSetLayoutBinding> GetVKDescriptorSetLayoutBinding() override;
	vector<VkDeviceSize> GetVKBufferSize() override;
	vector<VkDescriptorPoolSize> GetVKDescriptorSize();
	vector<VkWriteDescriptorSet> GetVKWriteDescriptorSet(size_t);
	ShaderType GetShaderType() override;
};