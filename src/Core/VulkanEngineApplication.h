#pragma once
/*
* Vulkan 系統的 Application
*/
#include <iostream>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

using namespace std;

// 測試顯卡的一些細節使用
#define VKENGINE_DEBUG_DETAILS

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

	// Vulkan
	VkInstance instance;
	VkPhysicalDevice physiclaDevice									= VK_NULL_HANDLE;
	int queueFamilyIndex											= -1;
	VkDevice device;
	VkQueue graphicsQueue;

	//////////////////////////////////////////////////////////////////////////
	// Helper Function
	//////////////////////////////////////////////////////////////////////////
	void __CreateVKInstance();																				// Vulkan Instance
	void __PickPhysicalDevice();																			// 選擇顯卡
	int __GetQueueIndexIfDeviceSuitable(VkPhysicalDevice);													// 是否為合適的顯卡 (是根據 Queue Family 是否支援 Graphics 來做判斷，大於等於 0 代表是合適的裝置)
	void __CreateLogicalDevice();																			// 根據對應的顯卡，去建立 Logical Device Interface
};

