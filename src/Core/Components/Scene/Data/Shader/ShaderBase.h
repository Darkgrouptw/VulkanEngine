#pragma once
#include "Core/Common/Common.h"
#include "Core/Components/Scene/Data/Shader/ShaderType.h"
#include "Core/Components/Scene/Data/ObjectBase.h"
#include "Core/Components/Scene/Data/VulkanInterface.h"
#include "Core/Components/Buffer/VertexBufferInfo.h"
#include "Core/Components/Buffer/UniformBufferInfo.h"

using namespace std;

class ShaderBase : public ObjectBase, VulkanInterface
{
public:
    ShaderBase(string);
    virtual ~ShaderBase() = default;

    // Vulkan Stuff
    void CreateVulkanStuff() override;
    void DestroyVulkanStuff() override;
    void BindGraphicsPipeline(const VkCommandBuffer);

    // 設定 Uniform Buffer
    void SetMVPUniformBuffer(const glm::mat4, const glm::mat4, const glm::mat4);

    // Get Vulkan Item
    inline VkPipelineLayout GetPipelineLayout()                     { return mPipelineLayout; };
    inline VkDescriptorSet& GetCurrentDescriptorSet()               { return mDescriptorSets[__GetCurrentFrameIndex()]; };

protected:
    virtual vector<VkDescriptorSetLayoutBinding> GetVKDescriptorSetLayoutBinding() = 0;                     // 要建立 Layout 的時候使用
    virtual vector<VkDeviceSize> GetVKBufferSize() = 0;                                                     // 拿總共需要件多少 Buffer Size
    virtual vector<VkDescriptorPoolSize> GetVKDescriptorSize() = 0;                                         // 建立 Descriptor Pool Size
    virtual vector<VkWriteDescriptorSet> GetVKWriteDescriptorSet(size_t) = 0;                               // 建立 WriteDescriptorSet
    virtual ShaderType GetShaderType() = 0;

    // 共同設定給下方人用的 Function
    vector<VkDescriptorSetLayoutBinding> CommonSetupForGetVKDescriptorSetLayoutBinding();
    vector<VkDeviceSize> CommonSetupForGetVKBufferSize();
    vector<VkDescriptorPoolSize> CommonSetupForGetVKDescriptorSize();
    vector<VkWriteDescriptorSet> CommonSetupForGetVKWriteDescriptorSet(size_t);

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
    vector<vector<VkBuffer>> mUniformBufferList;
    vector<vector<VkDeviceMemory>> mUniformBufferMemoryList;
    vector<vector<void*>> mUniformBufferMappedDataList;

    // Vulkan Descriptor
    VkDescriptorPool mDescriptorPool                                = NULL;
    vector<VkDescriptorSet> mDescriptorSets;
};
