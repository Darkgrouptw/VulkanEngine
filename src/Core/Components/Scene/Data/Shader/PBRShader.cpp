#include "Core/Components/Scene/Data/Shader/PBRShader.h"
#include "Core/VulkanEngineApplication.h"
using VKHelper = VulkanEngineApplication;

PBRShader::PBRShader() : ShaderBase(string(magic_enum::enum_name(GetShaderType()).data()))
{
}
PBRShader::~PBRShader()
{
}

#pragma region Public
// 設定 Uniform Buffer
void PBRShader::SetMatUniformBuffer(const glm::vec4 pAmbient, const glm::vec4 pDiffuse, const glm::vec4 pSpecular)
{
	MaterialBufferInfo bufferInfo{ .AmbientColor = pAmbient, .DiffuseColor = pDiffuse, .SpecularColor = pSpecular };
	memcpy(mUniformBufferMappedDataList[1][VKHelper::Instance->GetCurrentFrameIndex()], &bufferInfo, sizeof(MaterialBufferInfo));
}
#pragma endregion
#pragma region Protected
vector<VkDescriptorSetLayoutBinding> PBRShader::GetVKDescriptorSetLayoutBinding()
{
	//
	VkDescriptorSetLayoutBinding matLayout{};
	matLayout.binding 												= 1;
	matLayout.descriptorType										= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;	// Uniform Buffer
	matLayout.descriptorCount										= 1;
	matLayout.stageFlags											= VK_SHADER_STAGE_VERTEX_BIT;			// 使用於 Vertex Buffer 的 Uniform Buffer

	vector<VkDescriptorSetLayoutBinding> bindings					= CommonSetupForGetVKDescriptorSetLayoutBinding();
	bindings.push_back(matLayout);
	return bindings;
}
vector<VkDeviceSize> PBRShader::GetVKBufferSize()
{
    vector<VkDeviceSize> bufferSize                                 = CommonSetupForGetVKBufferSize();
    bufferSize.push_back(sizeof(MaterialBufferInfo));
    return bufferSize;
}
vector<VkDescriptorPoolSize> PBRShader::GetVKDescriptorSize()
{
	vector<VkDescriptorPoolSize> poolSizes                          = CommonSetupForGetVKDescriptorSize();
    VkDescriptorPoolSize matPoolSize{};
    matPoolSize.type												= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    matPoolSize.descriptorCount										= VKHelper::MAX_FRAME_IN_FLIGHTS;
	poolSizes.push_back(matPoolSize);
    return poolSizes;
}
vector<VkWriteDescriptorSet> PBRShader::GetVKWriteDescriptorSet(size_t pFrameIndex)
{
	VkDescriptorBufferInfo* bufferinfo								= new VkDescriptorBufferInfo{};
	bufferinfo->buffer												= mUniformBufferList[1][pFrameIndex];
	bufferinfo->offset												= 0;
	bufferinfo->range												= sizeof(MaterialBufferInfo);
	
    vector<VkWriteDescriptorSet> descriptorWrites                   = CommonSetupForGetVKWriteDescriptorSet(pFrameIndex);
    VkWriteDescriptorSet descriptorSet{};
	descriptorSet.sType											    = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorSet.dstSet											= mDescriptorSets[pFrameIndex];
	descriptorSet.dstBinding										= 1;
	descriptorSet.dstArrayElement								    = 0;

	descriptorSet.descriptorType                                    = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorSet.descriptorCount                                   = 1;
	descriptorSet.pBufferInfo                                       = bufferinfo;
	descriptorWrites.push_back(descriptorSet);
	return descriptorWrites;
}
ShaderType PBRShader::GetShaderType()
{
    return ShaderType::PBR;
}
#pragma endregion