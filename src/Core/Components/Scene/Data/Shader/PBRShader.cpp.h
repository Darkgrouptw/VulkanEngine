#pragma once
#include "Core/Components/Scene/Data/Shader/ShaderBase.h"
#include "Core/Components/Buffer/MaterialBufferInfo.h"

class PBRShader : public ShaderBase
{
public:
	PBRShader();
	~PBRShader();

	// 設定 Uniform Buffer
	void SetMatUniformBuffer(const glm::vec4, const glm::vec4, const glm::vec4);

protected:
	vector<VkDescriptorSetLayoutBinding> GetVKDescriptorSetLayoutBinding() override;
	vector<VkDeviceSize> GetVKBufferSize() override;
	vector<VkDescriptorPoolSize> GetVKDescriptorSize();
	vector<VkWriteDescriptorSet> GetVKWriteDescriptorSet(size_t);
	ShaderType GetShaderType() override;
};