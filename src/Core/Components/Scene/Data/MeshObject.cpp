#include "Core/Components/Scene/Data/MeshObject.h"
#include "Core/VulkanEngineApplication.h"
using VKHelper = VulkanEngineApplication;

#pragma region Public
MeshObject::MeshObject(string pName, int pIndex) : ObjectBase(pName)
{
	mMaterialIndex													= pIndex;
}
MeshObject::~MeshObject()
{
	mVertices.clear();
	mFaceIndices.clear();
}

// Insert Method
void MeshObject::InsertVertexData(glm::vec3 pPos, glm::vec3 pNormal, glm::vec4 pVertexColor, glm::vec2 pTexcoord)
{
	// ToDo:: Add Transform Matrix
	mVertices.push_back({ pPos, pNormal, pVertexColor, pTexcoord });
}
void MeshObject::InsertFaceIndex(uint32_t indices)
{
	mFaceIndices.push_back(indices);
}

// Get
uint32_t MeshObject::GetFaceIndicesSize()
{
	return mFaceIndices.size();
}
glm::mat4 MeshObject::GetModelMatrix()
{
	glm::mat4 mat;
	mat = glm::translate(mat, Position);
	mat = glm::scale(mat * glm::mat4_cast(Rotation), Scale);
	return mat;
}

// Vulkan Stuff
void MeshObject::CreateVulkanStuff()
{
	CreateVertexBuffer();
	CreateIndexBuffer();
}
void MeshObject::DestroyVulkanStuff()
{
	DestroyVertexBuffer();
	DestroyIndexBuffer();
}
void MeshObject::Render(const VkCommandBuffer pCommandBuffer, const VkPipelineLayout pPipelineLayout, const VkDescriptorSet& pDescriptorSet)
{
	// 送 Buffer 上去
	VkBuffer buffers[]											= { mVertexBuffer };
	VkDeviceSize offsets[]										= { 0 };
	vkCmdBindVertexBuffers(pCommandBuffer, 0, 1, buffers, offsets);
	vkCmdBindIndexBuffer(pCommandBuffer, mIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(pCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pPipelineLayout, 0, 1, &pDescriptorSet, 0, nullptr);
	vkCmdDrawIndexed(pCommandBuffer, static_cast<uint32_t>(mFaceIndices.size()), 1, 0, 0, 0);

	// Draw Vertex
	//vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	//vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
}
#pragma endregion
#pragma region Protected
// Vulkan Command
void MeshObject::CreateVertexBuffer()
{
	#pragma region 建立 Stage Buffer
	// 這裡會分成兩個 Buffer 的原因
	// 主要是因為資料上傳後就不會遭受到修改
	// 所以理想上會使用 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT (也就是無法從 CPU 去讀取)
	VkDeviceSize bufferSize											= sizeof(VertexBufferInfo) * mVertices.size();

	VkBuffer stageBuffer;
	VkDeviceMemory stageBufferMemory;

	VKHelper::Instance->CreateBuffer(
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 																	// 此 Buffer 可以當作 Memory transfer operation 的 source
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,							// 需要開啟這兩個 tag 才可以從 CPU 送上資料到 GPU
		stageBuffer,
		stageBufferMemory
	);
	#pragma endregion
	#pragma region Mapping 到 GPU
	void* data;
	vkMapMemory(VKHelper::Instance->GetDevice(), stageBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, mVertices.data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(VKHelper::Instance->GetDevice(), stageBufferMemory);
	#pragma endregion
	#pragma region Copy Buffer 只給 GPU 用
	VKHelper::Instance->CreateBuffer(
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 								// Memory Buffer Dst & Vertex Buffer Usage
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,																// Local 的 Memory (不需上傳)
		mVertexBuffer,
		mVertexBufferMemory);
	VKHelper::Instance->CopyBuffer(stageBuffer, mVertexBuffer, bufferSize);
	#pragma endregion
	#pragma region Destroy Stage Buffer
	vkDestroyBuffer(VKHelper::Instance->GetDevice(), stageBuffer, nullptr);
	vkFreeMemory(VKHelper::Instance->GetDevice(), stageBufferMemory, nullptr);
	#pragma endregion
}
void MeshObject::CreateIndexBuffer()
{
	#pragma region 建立 Stage Buffer
	VkDeviceSize bufferSize											= sizeof(uint32_t) * mFaceIndices.size();

	VkBuffer stageBuffer;
	VkDeviceMemory stageBufferMemory;

	VKHelper::Instance->CreateBuffer(
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 																	// 此 Buffer 可以當作 Memory transfer operation 的 source
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,							// 需要開啟這兩個 tag 才可以從 CPU 送上資料到 GPU
		stageBuffer,
		stageBufferMemory
	);
	#pragma endregion
	#pragma region Mapping 到 GPU
	void* data;
	vkMapMemory(VKHelper::Instance->GetDevice(), stageBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, mFaceIndices.data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(VKHelper::Instance->GetDevice(), stageBufferMemory);
	#pragma endregion
	#pragma region Copy Buffer 只給 GPU 用
	VKHelper::Instance->CreateBuffer(
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 								// Memory Buffer Dst & Index Buffer Usage
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,																// Local 的 Memory (不需上傳)
		mIndexBuffer,
		mIndexBufferMemory);
	VKHelper::Instance->CopyBuffer(stageBuffer, mIndexBuffer, bufferSize);
	#pragma endregion
	#pragma region Destroy Stage Buffer
	vkDestroyBuffer(VKHelper::Instance->GetDevice(), stageBuffer, nullptr);
	vkFreeMemory(VKHelper::Instance->GetDevice(), stageBufferMemory, nullptr);
	#pragma endregion
}
void MeshObject::DestroyVertexBuffer()
{
	vkDestroyBuffer(VKHelper::Instance->GetDevice(), mVertexBuffer, nullptr);
	vkFreeMemory(VKHelper::Instance->GetDevice(), mVertexBufferMemory, nullptr);
}
void MeshObject::DestroyIndexBuffer()
{
	vkDestroyBuffer(VKHelper::Instance->GetDevice(), mIndexBuffer, nullptr);
	vkFreeMemory(VKHelper::Instance->GetDevice(), mIndexBufferMemory, nullptr);
}
#pragma endregion