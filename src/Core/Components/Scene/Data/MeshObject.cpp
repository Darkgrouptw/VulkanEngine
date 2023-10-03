#include "Core/Components/Scene/Data/MeshObject.h"

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
	// 這裡會分成兩個 Buffer 的原因
	// 主要是因為資料上傳後就不會遭受到修改
	// 所以理想上會使用 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT (也就是無法從 CPU 去讀取)
	VkDeviceSize bufferSize											= sizeof(VertexBufferInfo) * mVertices.size();
}
void MeshObject::DestroyVertexBuffer()
{

}
#pragma endregion
