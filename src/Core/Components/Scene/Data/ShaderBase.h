#pragma once
#include "Core/Common/Common.h"
#include "Core/Components/Scene/Data/ShaderType.h"
#include "Core/Components/Scene/Data/ShaderTypeUtils.hpp"
#include "Core/Components/Scene/Data/ObjectBase.h"
#include "Core/Components/Scene/Data/VulkanInterface.h"

using namespace std;

class ShaderBase : public ObjectBase, VulkanInterface
{
public:
    ShaderBase(const ShaderType);
    ~ShaderBase();

    // Vulkan Stuff
    void CreateVulkanStuff() override;
    void DestroyVulkanStuff() override;

protected:
    ShaderType mType;

	// Vulkan Command
	void CreateDescriptorSetLayout();                                                                       // 在建立 GraphicsPipeline 前，要設定好 Uniform Buffer 的設定
	void CreateGraphicsPipeline();                                                                          // 建立 Graphics Pipeline
    void CreateUniformBuffer();                                                                             // 建立 Uniform Buffer

	void DestroyDescriptorSetLayout();
	void DestroyGraphicsPipeline();
    void DestroyUniformBuffer();

    // Helper Function
    vector<char> __ReadShaderFile(const string&);															// 讀取 ShaderFile
    VkShaderModule __CreateShaderModule(const vector<char>&);												// 產生 Shader Module

    // Vulkan Pipeline
    VkDescriptorSetLayout mDescriptorSetLayout;
    VkPipelineLayout mPipelineLayout;
    VkPipeline mGraphicsPipeline;


    // Vulkan Uniform Buffer
    vector<VkBuffer> mUniformBufferList;
    vector<VkDeviceMemory> mUniformBufferMemoryList;
    vector<void*> mUniformBufferMappedDataList;
};
