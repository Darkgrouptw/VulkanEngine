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
void MeshObject::InsertVertexData(glm::vec3 pPos, glm::vec3 pNormal, glm::vec2 pTexcoord, glm::vec3 pVertexColor)
{
	// ToDo:: Add Transform Matrix
	mVertices.push_back({ pPos, pNormal, pTexcoord, pVertexColor });
}
void MeshObject::InsertVertexData(glm::vec3 pPos, glm::vec3 pNormal, glm::vec2 pTexcoord)
{
	InsertVertexData(pPos, pNormal, glm::vec2(), glm::vec3());
}
void MeshObject::InsertVertexData(glm::vec3 pPos, glm::vec3 pNormal)
{
	InsertVertexData(pPos, pNormal, glm::vec2(), glm::vec3());
}
void MeshObject::InsertVertexData(glm::vec3 pPos)
{
	InsertVertexData(pPos, glm::vec3(), glm::vec2(), glm::vec3());
}
void MeshObject::InsertFaceIndex(int indices)
{
	mFaceIndices.push_back(indices);
}
void MeshObject::SetMaterialIndex(int pIndex)
{
	mMaterialIndex = pIndex;
}

// Get
uint32_t MeshObject::GetFaceIndicesSize()
{
	return mFaceIndices.size();
}


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
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 								// Memory Buffer Dst & VertexBuffer Usage
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
void MeshObject::DestroyVertexBuffer()
{
	vkDestroyBuffer(VKHelper::Instance->GetDevice(), mIndexBuffer, nullptr);
	vkDestroyBuffer(VKHelper::Instance->GetDevice(), mVertexBuffer, nullptr);
}
#pragma endregion
