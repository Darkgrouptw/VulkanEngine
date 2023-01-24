#pragma once
/*
* Vulkan 系統的 Application
*/
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class VulkanEngineApplication
{
public:
	void Run();


private:
	void InitWindow();																						// 初始化視窗
	void InitVulkan();																						// 初始化 Vulkan
	void MainLoop();																						// Main
	void Destroy();																							// 清空其他的資料

	// 視窗設定
	GLFWwindow* window												= NULL;									// GLFW Window
	const int WIDTH													= 1600;									// 長
	const int HEIGHT												= 900;									// 寬
};

