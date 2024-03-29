#include "Core/Components/Scene/Data/Shader/ShaderBase.h"
#include "Core/VulkanEngineApplication.h"
using VKHelper = VulkanEngineApplication;

ShaderBase::ShaderBase(string pName) : ObjectBase(pName)
{
}

#pragma region Public
// Vulkan Stuff
void ShaderBase::CreateVulkanStuff()
{
	CreateDescriptorSetLayout();
	CreateGraphicsPipeline();
	CreateUniformBuffer();
	CreateDescriptor();
}
void ShaderBase::DestroyVulkanStuff()
{
	DestroyDescriptor();
	DestroyUniformBuffer();
	DestroyGraphicsPipeline();
	DestroyDescriptorSetLayout();
}
void ShaderBase::BindGraphicsPipeline(const VkCommandBuffer pCommandBuffer)
{
	vkCmdBindPipeline(pCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);
}

// 設定 Uniform Buffer
void ShaderBase::SetMVPUniformBuffer(const glm::mat4 pProjM, const glm::mat4 pViewM, const glm::mat4 pModelM)
{
	MVPBufferInfo tempBuffer{ .ModelMatrix = pModelM, .ViewMatrix = pViewM, .ProjectionMatrix = pProjM };
	memcpy(mUniformBufferMappedDataList[0][VKHelper::Instance->GetCurrentFrameIndex()], &tempBuffer, sizeof(MVPBufferInfo));
}
#pragma endregion
#pragma region Protected
// 共同設定給下方人用的 Function
vector<VkDescriptorSetLayoutBinding> ShaderBase::CommonSetupForGetVKDescriptorSetLayoutBinding()
{
	VkDescriptorSetLayoutBinding uboLayout{};
	uboLayout.binding 												= 0;
	uboLayout.descriptorType										= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;	// Uniform Buffer
	uboLayout.descriptorCount										= 1;
	uboLayout.stageFlags											= VK_SHADER_STAGE_VERTEX_BIT;			// 使用於 Vertex Buffer 的 Uniform Buffer

	// ToDo: Add Texture
	//auto textureLayout												= TextM->CreateDescriptorSetLayout();
	//vector<VkDescriptorSetLayoutBinding> bindings					= { uboLayout, textureLayout };
	vector<VkDescriptorSetLayoutBinding> bindings;
	bindings.push_back(uboLayout);
	return bindings;
}
vector<VkDeviceSize> ShaderBase::CommonSetupForGetVKBufferSize()
{
	vector<VkDeviceSize> sizeList;
	sizeList.push_back(sizeof(MVPBufferInfo));
	return sizeList;
}
vector<VkDescriptorPoolSize> ShaderBase::CommonSetupForGetVKDescriptorSize()
{
	vector<VkDescriptorPoolSize> poolSizes;
	VkDescriptorPoolSize mvpPoolSize{};
	mvpPoolSize.type												= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	mvpPoolSize.descriptorCount										= VKHelper::MAX_FRAME_IN_FLIGHTS;
	poolSizes.push_back(mvpPoolSize);
	return poolSizes;
}
vector<VkWriteDescriptorSet> ShaderBase::CommonSetupForGetVKWriteDescriptorSet(size_t pFrameIndex)
{
	VkDescriptorBufferInfo *bufferinfo								= new VkDescriptorBufferInfo{};
	bufferinfo->buffer												= mUniformBufferList[0][pFrameIndex];
	bufferinfo->offset												= 0;
	bufferinfo->range												= sizeof(MVPBufferInfo);

	vector<VkWriteDescriptorSet> descriptorWrites;
	VkWriteDescriptorSet descriptorSet{};
	descriptorSet.sType												= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorSet.dstSet											= mDescriptorSets[pFrameIndex];
	descriptorSet.dstBinding										= 0;
	descriptorSet.dstArrayElement									= 0;

	descriptorSet.descriptorType									= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorSet.descriptorCount									= 1;
	descriptorSet.pBufferInfo										= bufferinfo;
	descriptorWrites.push_back(descriptorSet);
	/*
	//VkDescriptorImageInfo imageInfo								= TextM->CreateDescriptorImageInfo();

	vector<VkWriteDescriptorSet> descriptorWrites;
	descriptorWrites.resize(bufferList.size());
	#pragma region Uniform Buffer

	#pragma endregion
	#pragma region Image Info
	descriptorWrites[1].sType									= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].dstSet									= DescriptorSets[i];
	descriptorWrites[1].dstBinding								= 1;
	descriptorWrites[1].dstArrayElement							= 0;

	descriptorWrites[1].descriptorType							= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[1].descriptorCount							= 1;
	descriptorWrites[1].pImageInfo								= &imageInfo;
	#pragma endregion*/
	return descriptorWrites;
}

// Vulkan Create Command
void ShaderBase::CreateDescriptorSetLayout()
{
	vector<VkDescriptorSetLayoutBinding> bindings					= GetVKDescriptorSetLayoutBinding();
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
	ShaderType shaderType											= GetShaderType();
	vertexShader													= __ReadShaderFile(ShaderTypeUtils::GetVertexShaderPath(shaderType));
	fragmentShader													= __ReadShaderFile(ShaderTypeUtils::GetFragmentShaderPath(shaderType));
	
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
	// 6. Color Blending & Depth Stencil Test						Blending 到 FrameBuffer 中
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
	rasterizationInfo.polygonMode									= VK_POLYGON_MODE_FILL;					// 如果要只看 Wireframe，是在這裡設定
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
	#pragma region 6. Color Blending and Depth Stencil Test
	// 這裡要設定兩個
	// 1. 設定 Color Blending 後的 FrameBuffer 上的設定
	// 2. 設定 Global 的 Color Blending
	// 3. 設定 Global 的 Depth
	
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

	// 3.
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
	depthStencilInfo.sType											= VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilInfo.depthTestEnable								= VK_TRUE;
	depthStencilInfo.depthWriteEnable								= VK_TRUE;

	depthStencilInfo.depthCompareOp									= VK_COMPARE_OP_LESS_OR_EQUAL;

	// 是否要卡到 0 ~ 1 之間 （暫先不設定）
	depthStencilInfo.depthBoundsTestEnable							= VK_FALSE;
	depthStencilInfo.minDepthBounds									= 0.f;
	depthStencilInfo.maxDepthBounds									= 1.f;

	// Stencil Test
	depthStencilInfo.stencilTestEnable								= VK_FALSE;
	depthStencilInfo.front											= {};
	depthStencilInfo.back											= {};
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
	pipelineInfo.pColorBlendState									= &colorBlendInfo;
	pipelineInfo.pDepthStencilState									= &depthStencilInfo;
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
void ShaderBase::CreateUniformBuffer()
{
	#pragma region 填寫 Buffer Size
	vector<VkDeviceSize> bufferList									= GetVKBufferSize();
	#pragma endregion
	#pragma region Create Buffer
	int size														= bufferList.size();
	mUniformBufferList.resize(size);
	mUniformBufferMemoryList.resize(size);
	mUniformBufferMappedDataList.resize(size);
	for (int i = 0; i < size; i++)
	{
		VkDeviceSize bufferSize										= bufferList[i];

		mUniformBufferList[i].resize(VKHelper::MAX_FRAME_IN_FLIGHTS);
		mUniformBufferMemoryList[i].resize(VKHelper::MAX_FRAME_IN_FLIGHTS);
		mUniformBufferMappedDataList[i].resize(VKHelper::MAX_FRAME_IN_FLIGHTS);

		for (size_t j = 0; j < VKHelper::MAX_FRAME_IN_FLIGHTS; j++)
		{
			VKHelper::Instance->CreateBuffer(
				bufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				mUniformBufferList[i][j],
				mUniformBufferMemoryList[i][j]);

			vkMapMemory(VKHelper::Instance->GetDevice(), mUniformBufferMemoryList[i][j], 0, bufferSize, 0, &mUniformBufferMappedDataList[i][j]);
		}
	}
	#pragma endregion
}
void ShaderBase::CreateDescriptor()
{
	#pragma region Descriptor Pool
	VkDevice device													= VKHelper::Instance->GetDevice();
	vector<VkDeviceSize> bufferList									= GetVKBufferSize();
	vector<VkDescriptorPoolSize> poolSizes							= GetVKDescriptorSize();

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType													= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount											= poolSizes.size();
	poolInfo.pPoolSizes												= poolSizes.data();
	poolInfo.maxSets												= VKHelper::MAX_FRAME_IN_FLIGHTS;

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS)
		throw runtime_error("Failed to create descriptor pool");
	#pragma endregion
	#pragma region Descriptor Set
	vector<VkDescriptorSetLayout> layouts(VKHelper::MAX_FRAME_IN_FLIGHTS, mDescriptorSetLayout);

	VkDescriptorSetAllocateInfo allocateInfo{};
	allocateInfo.sType												= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocateInfo.descriptorPool										= mDescriptorPool;
	allocateInfo.descriptorSetCount									= VKHelper::MAX_FRAME_IN_FLIGHTS;
	allocateInfo.pSetLayouts										= layouts.data();

	mDescriptorSets.resize(VKHelper::MAX_FRAME_IN_FLIGHTS);
	if (vkAllocateDescriptorSets(device, &allocateInfo, mDescriptorSets.data()) != VK_SUCCESS)
		throw runtime_error("Failed to create allocate descriptor set");

	for (size_t i = 0; i < VKHelper::MAX_FRAME_IN_FLIGHTS; i++)
	{
		vector<VkWriteDescriptorSet> descriptorWrites				= GetVKWriteDescriptorSet(i);
		vkUpdateDescriptorSets(device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);

		// Delete Buffer Info
		for (int j = 0; j < descriptorWrites.size(); j++)
			delete descriptorWrites[j].pBufferInfo;
	}
	#pragma endregion
}

// Vulkan Destroy Command
void ShaderBase::DestroyDescriptorSetLayout()
{
	vkDestroyDescriptorSetLayout(VKHelper::Instance->GetDevice(), mDescriptorSetLayout, nullptr);
}
void ShaderBase::DestroyGraphicsPipeline()
{
	vkDestroyPipeline(VKHelper::Instance->GetDevice(), mGraphicsPipeline, nullptr);
}
void ShaderBase::DestroyUniformBuffer()
{
	VkDevice device = VKHelper::Instance->GetDevice();
	vector<VkDeviceSize> bufferList = GetVKBufferSize();
	for(size_t i = 0; i < bufferList.size(); i++)
		for (size_t j = 0; j < VKHelper::MAX_FRAME_IN_FLIGHTS; j++)
		{
			vkDestroyBuffer(device, mUniformBufferList[i][j], nullptr);
			vkFreeMemory(device, mUniformBufferMemoryList[i][j], nullptr);
		}
}
void ShaderBase::DestroyDescriptor()
{
	VkDevice device = VKHelper::Instance->GetDevice();
	vkDestroyDescriptorPool(device, mDescriptorPool, nullptr);
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
uint32_t ShaderBase::__GetCurrentFrameIndex()
{
	return VKHelper::Instance->GetCurrentFrameIndex();
}
#pragma endregion