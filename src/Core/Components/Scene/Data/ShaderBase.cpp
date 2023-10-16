#include "Core/Components/Scene/Data/ShaderBase.h"
#include "Core/VulkanEngineApplication.h"
using VKHelper = VulkanEngineApplication;

ShaderBase::ShaderBase(string pName, ShaderType pType) : ObjectBase(pName)
{
	mType = pType;
}
ShaderBase::~ShaderBase()
{
}

#pragma region Public
void ShaderBase::CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayout{};
	uboLayout.binding 												= 0;
	uboLayout.descriptorType										= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;	// Uniform Buffer
	uboLayout.descriptorCount										= 1;
	uboLayout.stageFlags											= VK_SHADER_STAGE_VERTEX_BIT;			// 使用於 Vertex Buffer 的 Uniform Buffer

	// ToDo: Add Texture
	//auto textureLayout												= TextM->CreateDescriptorSetLayout();
	//vector<VkDescriptorSetLayoutBinding> bindings					= { uboLayout, textureLayout };
	vector<VkDescriptorSetLayoutBinding> bindings					= { uboLayout };
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType												= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount											= bindings.size();
	layoutInfo.pBindings											= bindings.data();

	if (vkCreateDescriptorSetLayout(VKHelper::Instance->GetDevice(), &layoutInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS)
		throw runtime_error("Failed to create descriptor set layout");
}
void ShaderBase::CreateGraphicsPipeline()
{
	vector<char> vertexShader, fragmentShader;
	//vertexShader = __ReadShaderFile
}
#pragma endregion
#pragma region Private
vector<char> ShaderBase::ReadShaderFile(const string& path)
{
	// 讀取檔案
	// ate: 從檔案的結果開始讀（主要是判斷檔案的大小，之後還可以移動此指標到想要的位置）
	// binary: 避免 text transformations
	ifstream file(path, ios::ate | ios::binary);
	if (!file.is_open())
		throw runtime_error("Failed to open the file(" + path + ")");

	size_t fileSize = file.tellg();
	vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();
	return buffer;
}
#pragma endregion