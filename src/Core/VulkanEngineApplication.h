#pragma once
/*
* Vulkan 系統的 Application
*/
#include <iostream>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

using namespace std;

// 測試 Vulkan 的一些細節使用
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
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;
	VkPhysicalDevice physiclaDevice									= VK_NULL_HANDLE;
	int queueFamilyIndex											= -1;
	VkDevice device;
	VkQueue graphicsQueue;

	//////////////////////////////////////////////////////////////////////////
	// Helper Function
	//////////////////////////////////////////////////////////////////////////
	void __CreateVKInstance();																				// Vulkan Instance
	void __SetupDebugMessenger();																			// 設定 Vulkan 的 Debug 工具
	void __PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT&);							// 設定 Type
	void __CreateSurface();																					// 建立和視窗溝通的 Surface (GLFW & Vulkan)
	void __PickPhysicalDevice();																			// 選擇顯卡
	int __GetQueueIndexIfDeviceSuitable(VkPhysicalDevice);													// 是否為合適的顯卡 (是根據 Queue Family 是否支援 Graphics 來做判斷，大於等於 0 代表是合適的裝置)
	void __CreateLogicalDevice();																			// 根據對應的顯卡，去建立 Logical Device Interface

	// Debug Vulkan 的功能
#if defined(VKENGINE_DEBUG_DETAILS)
	bool EnabledValidationLayer										= false;								// 是否底層有支援這個功能
	vector<const char*> ValidationLayersNames 						= 										// 判斷是否有底下的標籤，就代表是否可以開啟這個功能
	{
		"VK_LAYER_KHRONOS_validation"
	};
	bool __CheckValidationLayerSupport();																	// 檢查 Vulkan Validtion Layer 是否支援
#endif
};

