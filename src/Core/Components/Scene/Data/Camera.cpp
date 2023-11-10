#include "Core/Components/Scene/Data/Camera.h"
#include "Core/VulkanEngineApplication.h"
using VKHelper = VulkanEngineApplication;

#pragma region Public
Camera::Camera(string pName) : ObjectBase(pName)
{
	Position = glm::vec3(0.f, 0.f, 10.f);
}
Camera::~Camera()
{
}

glm::mat4 Camera::GetProjectMatrix()
{
	glm::mat4 projectM = glm::perspective(glm::radians(mFov), mAspect, mNear, mFar);

	// 這裡必須要反轉
	// 因為 GLM 是針對 OpenGL 做的 (y 的方向是相反的)
	projectM[1][1]													*= -1;
	return projectM;
}
#pragma endregion
#pragma region Protected
#pragma endregion