#include "Core/Components/Scene/Data/MeshObject.h"

#pragma region Public
MeshObject::MeshObject(string pName, int pIndex) : ObjectBase(pName)
{
	mMaterialIndex													= pIndex;
}
MeshObject::~MeshObject()
{
	mPositions.clear();
	mNormals.clear();
	mFaceIndices.clear();
}

// Insert Method
void MeshObject::InsertPositionAndNormal(glm::vec3 pPos, glm::vec3 pNormal)
{
	InsertPosition(pPos);
	mNormals.push_back(pNormal);
}
void MeshObject::InsertPosition(glm::vec3 pPos)
{
	mPositions.push_back(pPos);
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
#pragma endregion
