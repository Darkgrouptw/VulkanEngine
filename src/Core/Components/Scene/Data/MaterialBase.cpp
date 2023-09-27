#include "Core/Components/Scene/Data/MaterialBase.h"

MaterialBase::MaterialBase(string pName) : ObjectBase(pName)
{
}
MaterialBase::~MaterialBase()
{
}


#pragma region Public
// Get
glm::vec3 MaterialBase::GetAmbientColor()
{
	return mAmbientColor;
}
glm::vec3 MaterialBase::GetDiffuseColor()
{
	return mDiffuseColor;
}
glm::vec3 MaterialBase::GetSpecularColor()
{
	return mSpecularColor;
}

// Set
void MaterialBase::SetAmbientColor(glm::vec3 pColor)
{
	mAmbientColor = pColor;
}
void MaterialBase::SetDiffuseColor(glm::vec3 pColor)
{
	mDiffuseColor = pColor;
}
void MaterialBase::SetSpecularColor(glm::vec3 pColor)
{
	mSpecularColor = pColor;
}
#pragma endregion
