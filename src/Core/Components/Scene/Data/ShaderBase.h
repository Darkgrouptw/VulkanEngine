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
    void BindGraphicsPipeline(const VkCommandBuffer);

    // Get Vulkan Item
    inline VkPipelineLayout GetPipelineLayout() { return mPipelineLayout; };
    inline VkDescriptorSet GetCurrentDescriptorSet() { return mDescriptorSets[__GetCurrentFrameIndex()]; };

protected:
    ShaderType mType;

	// Vulkan Create Command
	void CreateDescriptorSetLayout();                                                                       // 在建立 GraphicsPipeline 前，要設定好 Uniform Buffer 的設定
	void CreateGraphicsPipeline();                                                                          // 建立 Graphics Pipeline
    void CreateUniformBuffer();                                                                             // 建立 Uniform Buffer
    void CreateDescriptor();                                                                                // 建立 Descriptor Pool & Set (給 Unifrom Buffer 用)

    // Vulkan Destroy Command
	void DestroyDescriptorSetLayout();
	void DestroyGraphicsPipeline();
    void DestroyUniformBuffer();
    void DestroyDescriptor();

    // Helper Function
    vector<char> __ReadShaderFile(const string&);															// 讀取 ShaderFile
    VkShaderModule __CreateShaderModule(const vector<char>&);												// 產生 Shader Module
    uint32_t __GetCurrentFrameIndex();                                                                      // 拿要現在的 FrameIndex

    // Vulkan Pipeline
    VkDescriptorSetLayout mDescriptorSetLayout                      = NULL;
    VkPipelineLayout mPipelineLayout                                = NULL;
    VkPipeline mGraphicsPipeline                                    = NULL;

    // Vulkan Uniform Buffer
    vector<VkBuffer> mUniformBufferList;
    vector<VkDeviceMemory> mUniformBufferMemoryList;
    vector<void*> mUniformBufferMappedDataList;

    // Vulkan Descriptor
    VkDescriptorPool mDescriptorPool                                = NULL;
    vector<VkDescriptorSet> mDescriptorSets;
};
