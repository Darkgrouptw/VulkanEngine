#include "Core/Components/Scene/Data/MeshObject.h"

#pragma region Public
MeshObject::MeshObject(string name, int pIndex)
{
	mName															= name;
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

// Get
uint32_t MeshObject::GetFaceIndicesSize()
{
	return mFaceIndices.size();
}
#pragma endregion
