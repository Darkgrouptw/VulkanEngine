#include "Core/Components/Scene/Data/ShaderBase.h"
#include "Core/VulkanEngineApplication.h"
using VKHelper = VulkanEngineApplication;

ShaderBase::ShaderBase(const ShaderType pType) : ObjectBase(string(magic_enum::enum_name(pType).data()))
{
	mType = pType;
}
ShaderBase::~ShaderBase()
{
}

// Vulkan Stuff
void ShaderBase::CreateVulkanStuff()
{
	CreateDescriptorSetLayout();
	CreateGraphicsPipeline();
}
void ShaderBase::DestroyVulkanStuff()
{
	DestroyGraphicsPipeline();
	DestroyDescriptorSetLayout();
}
#pragma region Public

#pragma endregion
#pragma region Protected

// Vulkan Command
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
	// 讀取檔案並建立 Shader Module
	#pragma region VkShaderModule
	vector<char> vertexShader, fragmentShader;
	vertexShader													= __ReadShaderFile(ShaderTypeUtils::GetVertexShaderPath(mType));
	fragmentShader													= __ReadShaderFile(ShaderTypeUtils::GetFragmentShaderPath(mType));
	
	VkShaderModule vertexModule										= __CreateShaderModule(vertexShader);
	VkShaderModule fragmentModule									= __CreateShaderModule(fragmentShader);
	
	// 產生 Graphics Pipeline
	const char* mainFunctionName 									= "main"; 								// 入口的 Function 名稱
	VkPipelineShaderStageCreateInfo vertexStageCreateInfo{};
	vertexStageCreateInfo.sType										= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexStageCreateInfo.stage										= VK_SHADER_STAGE_VERTEX_BIT;
	vertexStageCreateInfo.module									= vertexModule;
	vertexStageCreateInfo.pName										= mainFunctionName;

	VkPipelineShaderStageCreateInfo fragmentStageCreateInfo{};
	fragmentStageCreateInfo.sType									= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentStageCreateInfo.stage									= VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentStageCreateInfo.module									= fragmentModule;
	fragmentStageCreateInfo.pName									= mainFunctionName;
	
	// 創建 Shader Stage
	VkPipelineShaderStageCreateInfo shaderStages[]					= { vertexStageCreateInfo, fragmentStageCreateInfo };
	#pragma endregion
	#pragma region Graphics Pipeline Layout
	// Graphics Pipeline Layout:
	// 1. Vertex Input												點的資料
	// 2. Input Assembler 											點怎麼連接
	// 3. Vertex Shader	(From Code)											
	// 4. Rasterization & MultiSamping
	// 5. Fragment Shader (Fom Code)
	// 6. Color Blending											Blending 到 FrameBuffer 中
	// 7. FrameBuffer
	#pragma region 1. Vertex Input
	auto attribeDesc												= VertexBufferInfo::GetAttributeDescription();
	auto bindingDesc												= VertexBufferInfo::GetBindingDescription();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType											= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexAttributeDescriptionCount					= static_cast<uint32_t>(attribeDesc.size());
	vertexInputInfo.pVertexAttributeDescriptions					= attribeDesc.data();
	vertexInputInfo.vertexBindingDescriptionCount					= 1;
	vertexInputInfo.pVertexBindingDescriptions						= &bindingDesc;
	#pragma endregion
	#pragma region 2. Input Assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
	inputAssemblyInfo.sType											= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyInfo.topology										= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;	// 每三個點使用後，不複用
	inputAssemblyInfo.primitiveRestartEnable						= VK_FALSE;								// 上方為 _STRIP 才會使用
	#pragma endregion
	#pragma region Viewport & Scissor
	// Generate Viewport & Scissor
	VkViewport viewport{};
	VkRect2D scissor{};
	VKHelper::Instance->GetViewportAndScissor(viewport, scissor);

	// Viewport
	VkPipelineViewportStateCreateInfo viewportInfo{};
	viewportInfo.sType												= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportInfo.viewportCount										= 1;
	viewportInfo.scissorCount										= 1;
	viewportInfo.pViewports											= &viewport;
	viewportInfo.pScissors											= &scissor;
	#pragma endregion
	#pragma region 5. Rasterization & MultiSamping
	VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
	rasterizationInfo.sType											= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationInfo.depthClampEnable								= VK_FALSE;								// 對於 Shadow Map 可能會有需要 Near 到 Far 之間的資訊，其他都不需要設為 True
	rasterizationInfo.rasterizerDiscardEnable						= VK_FALSE;								// 如果設定成 True，會造成 geometry 不會傳入 rasterization
	rasterizationInfo.cullMode										= VK_CULL_MODE_BACK_BIT;				// 去除 Back face
	rasterizationInfo.frontFace										= VK_FRONT_FACE_CLOCKWISE;				// 逆時針的 Vertex，算 Front Face
	rasterizationInfo.depthBiasEnable								= VK_FALSE;
	rasterizationInfo.lineWidth										= 1.0;
	// 當設定為 False，底下設定就不需要設定
	//rasterizationInfo.depthBiasConstantFactor 						= 0.0f;
	//rasterizationInfo.depthBiasClamp 								= 0.0f;
	//rasterizationInfo.depthBiasSlopeFactor 							= 0.0f;

	// Multisampling (Anti-Aliasing)
	VkPipelineMultisampleStateCreateInfo multisamplingInfo{};
	multisamplingInfo.sType											= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingInfo.rasterizationSamples							= VK_SAMPLE_COUNT_1_BIT;
	multisamplingInfo.sampleShadingEnable							= VK_FALSE;
	// 上方要設定為 True，下方才要設定
	//multisamplingInfo.minSampleShading 								= 1.0f;
	//multisamplingInfo.pSampleMask 									= nullptr;
	//multisamplingInfo.alphaToCoverageEnable							= VK_FALSE;
	//multisamplingInfo.alphaToOneEnable 								= VK_FALSE;
	#pragma endregion
	#pragma region 6. Color Blending
	// 這裡要設定兩個
	// 1. 設定 Color Blending 後的 FrameBuffer 上的設定
	// 2. 設定 Global 的 Color Blending
	
	// 1.
	VkPipelineColorBlendAttachmentState colorBlendAttachmentInfo{};
	colorBlendAttachmentInfo.colorWriteMask							= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachmentInfo.blendEnable							= VK_FALSE;
	// 開啟 Blend 才需要設定
	//colorBlendAttachmentInfo.srcColorBlendFactor 					= VK_BLEND_FACTOR_ONE; // Optional
	//colorBlendAttachmentInfo.dstColorBlendFactor 					= VK_BLEND_FACTOR_ZERO; // Optional
	//colorBlendAttachmentInfo.colorBlendOp 							= VK_BLEND_OP_ADD; // Optional
	//colorBlendAttachmentInfo.srcAlphaBlendFactor 					= VK_BLEND_FACTOR_ONE; // Optional
	//colorBlendAttachmentInfo.dstAlphaBlendFactor 					= VK_BLEND_FACTOR_ZERO; // Optional
	//colorBlendAttachmentInfo.alphaBlendOp 							= VK_BLEND_OP_ADD; // Optional	

	// 2.
	VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
	colorBlendInfo.sType											= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendInfo.logicOpEnable									= VK_FALSE;
	colorBlendInfo.attachmentCount									= 1;
	colorBlendInfo.pAttachments										= &colorBlendAttachmentInfo;
	#pragma endregion

	// 而在 OpenGL 中已經設定好 Fixed Function
	// 在 Vulkan 中要手動設定
	vector<VkDynamicState> states = {
		VK_DYNAMIC_STATE_VIEWPORT,									// 需要手動設定 vkCmdSetViewport
		VK_DYNAMIC_STATE_SCISSOR									// 需要手動設定 vkCmdSetScissor
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType												= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount									= static_cast<uint32_t>(states.size());
	dynamicState.pDynamicStates										= states.data();

	// 設定 Layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType										= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount								= 1;
	pipelineLayoutInfo.pSetLayouts									= &mDescriptorSetLayout;				// Uniform Layout
	pipelineLayoutInfo.pushConstantRangeCount						= 0;
	pipelineLayoutInfo.pPushConstantRanges							= nullptr;

	if (vkCreatePipelineLayout(VKHelper::Instance->GetDevice(), &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
		throw runtime_error("Failed to create pipeline");
	#pragma endregion
	#pragma region Graphics Pipeline
	// 建立 Graphics Pipeline
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType												= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount											= static_cast<uint32_t>(sizeof(shaderStages) / sizeof(VkPipelineShaderStageCreateInfo));
	pipelineInfo.pStages											= shaderStages;

	pipelineInfo.pVertexInputState									= &vertexInputInfo;
	pipelineInfo.pInputAssemblyState								= &inputAssemblyInfo;
	pipelineInfo.pViewportState										= &viewportInfo;
	pipelineInfo.pRasterizationState								= &rasterizationInfo;
	pipelineInfo.pMultisampleState									= &multisamplingInfo;
	pipelineInfo.pDepthStencilState									= nullptr;
	pipelineInfo.pColorBlendState									= &colorBlendInfo;
	pipelineInfo.pDynamicState										= &dynamicState;

	pipelineInfo.layout												= mPipelineLayout;
	pipelineInfo.renderPass											= VKHelper::Instance->GetRenderPass();
	pipelineInfo.subpass											= 0;
	pipelineInfo.basePipelineHandle									= VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(VKHelper::Instance->GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mGraphicsPipeline) != VK_SUCCESS)
		throw runtime_error("Failed to create Graphics Pipeline");
	#pragma endregion
	#pragma region Destroy Module
	vkDestroyShaderModule(VKHelper::Instance->GetDevice(), vertexModule, nullptr);
	vkDestroyShaderModule(VKHelper::Instance->GetDevice(), fragmentModule, nullptr);
	#pragma endregion
}
void ShaderBase::DestroyDescriptorSetLayout()
{
	vkDestroyDescriptorSetLayout(VKHelper::Instance->GetDevice(), mDescriptorSetLayout, nullptr);
}
void ShaderBase::DestroyGraphicsPipeline()
{
	vkDestroyPipeline(VKHelper::Instance->GetDevice(), mGraphicsPipeline, nullptr);
}

vector<char> ShaderBase::__ReadShaderFile(const string& path)
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
VkShaderModule ShaderBase::__CreateShaderModule(const vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType												= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize												= code.size();
	createInfo.pCode												= reinterpret_cast<const uint32_t*>(code.data());	// 要將原本的 char* 轉成 uint32_t*

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(VKHelper::Instance->GetDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		throw runtime_error("Failed to create shader module");
	return shaderModule;
}

#pragma endregion