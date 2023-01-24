#include "VulkanEngineApplication.h"


#pragma region Public
void VulkanEngineApplication::Run()
{
	InitWindow();
	InitVulkan();
	MainLoop();
	Destroy();
}
#pragma endregion
#pragma region Private

void VulkanEngineApplication::InitWindow()
{
	// 初始化
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);															// 預設初始化設定預設是 OpenGL，所以只用 NO_API
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);																// 設定不做 Resize

	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void VulkanEngineApplication::InitVulkan()
{

}

void VulkanEngineApplication::MainLoop()
{
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}
}

void VulkanEngineApplication::Destroy()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

#pragma endregion
