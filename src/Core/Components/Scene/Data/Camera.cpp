#include "Core/Components/Scene/Data/Camera.h"
#include "Core/VulkanEngineApplication.h"
using VKHelper = VulkanEngineApplication;

#pragma region Public
Camera::Camera(string pName) : ObjectBase(pName)
{
	Position = glm::vec3(0, 0, 10);
	mLookAtPos = glm::toMat4(Rotation) * Common::VectorForward;
}
Camera::~Camera()
{
}

#pragma endregion
#pragma region Protected
#pragma endregion