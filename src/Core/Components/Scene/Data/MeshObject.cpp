#include "Core/Components/Scene/Data/MeshObject.h"

#pragma region Public
MeshObject::MeshObject()
{
}
MeshObject::~MeshObject()
{
}

void MeshObject::InsertPositionAndNormal(glm::vec3 pPos, glm::vec2 pNormal)
{
	InsertPosition(pPos);
	mNormals.push_back(pNormal);
}
void MeshObject::InsertPosition(glm::vec3 pPos)
{
	mPositions.push_back(pPos);
}
#pragma endregion
