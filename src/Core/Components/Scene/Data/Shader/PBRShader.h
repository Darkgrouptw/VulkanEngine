#pragma once
#include "Core/Components/Scene/Data/Shader/ShaderBase.h"
#include "Core/Components/Buffer/SceneBufferInfo.h"
#include "Core/Components/Buffer/MaterialBufferInfo.h"

class PBRShader : public ShaderBase
{
public:
	PBRShader();
	~PBRShader();

	// 設定 Uniform Buffer
	void SetSceneUniformBuffer(const glm::vec3, const glm::vec3);
	void SetMatUniformBuffer(const glm::vec4, const glm::vec4, const glm::vec4);

protected:
	vector<VkDescriptorSetLayoutBinding> GetVKDescriptorSetLayoutBinding() override;
	vector<VkDeviceSize> GetVKBufferSize() override;
	vector<VkDescriptorPoolSize> GetVKDescriptorSize() override;
	vector<VkWriteDescriptorSet> GetVKWriteDescriptorSet(size_t) override;
	ShaderType GetShaderType() override;
};