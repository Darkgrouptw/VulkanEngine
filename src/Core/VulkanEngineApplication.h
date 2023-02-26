#pragma once
/*
* Vulkan 系統的 Application
*/
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <set>
#include <cstdint>
#include <limits>
#include <optional>
#include <fstream>
#include <algorithm>

using namespace std;

// 測試 Vulkan 的一些細節使用
#define VKENGINE_DEBUG_DETAILS

// 在 Vulkan 中，有很多不同的 Queue，分別各次處理不同的 operation
struct QueueFamilyIndices
{
	optional<uint32_t> GraphicsFamily;
	optional<uint32_t> PresentFamily;

	// 判斷是否有給過值
	bool IsCompleted()
	{
		return GraphicsFamily.has_value() && PresentFamily.has_value();
	}
};

// 檢查並建立 Swap Chain 的設定
struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR Capbilities;																	// 基本 Surface Capabilities (min/max number of images in swap chain, min/max width and height of images)
	vector<VkSurfaceFormatKHR> Formats;																		// Surface formats (Pixel format, Color Space) Ex: VK_FORMAT_B8G8R8A8_UNORM
	vector<VkPresentModeKHR> PresentModes;																	// 可以顯示的 Mode Ex: VK_PRESENT_MODE_FIFO_KHR
};

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
	GLFWwindow* Window												= NULL;									// GLFW Window
	const int WIDTH													= 1600;									// 長
	const int HEIGHT												= 900;									// 寬

	// Vulkan
	VkInstance Instance;
	VkDebugUtilsMessengerEXT DebugMessenger;
	VkSurfaceKHR Surface;
	VkPhysicalDevice PhysiclaDevice									= VK_NULL_HANDLE;
	VkDevice Device;
	VkPipelineLayout PipelineLayout;

	// Vulkan Queue
	VkQueue GraphicsQueue;
	VkQueue PresentQueue;

	// Vulkan SwapChain
	VkSwapchainKHR SwapChain;
	vector<VkImage> SwapChainImages;
	vector<VkImageView> SwapChainImageViews;
	VkFormat SwapChainImageFormat;
	VkExtent2D SwapChainExtent;

	//////////////////////////////////////////////////////////////////////////
	// Helper Function
	//////////////////////////////////////////////////////////////////////////
	void __CreateVKInstance();																				// Vulkan Instance
	void __SetupDebugMessenger();																			// 設定 Vulkan 的 Debug 工具
	void __CreateSurface();																					// 建立和視窗溝通的 Surface (GLFW & Vulkan)
	void __PickPhysicalDevice();																			// 選擇顯卡
	void __CreateLogicalDevice();																			// 根據對應的顯卡，去建立 Logical Device Interface
	void __CreateSwapChain();																				// 建立 Swap Chain
	void __CreateImageViews();																				// 建立 Image Views
	void __CreateGraphicsPipeline();																		// 建立 Graphics Pipeline
	
	//////////////////////////////////////////////////////////////////////////
	// 比較 Minor 的 Helper Function
	//////////////////////////////////////////////////////////////////////////
	void __PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT&);							// 設定 Type
	bool __CheckDeviceExtensionSupport(VkPhysicalDevice);													// 檢查 DeviceExtension 是否支援某幾項
	bool __IsDeviceSuitable(VkPhysicalDevice);																// 是否為合適的顯卡
	QueueFamilyIndices __FindQueueFamilies(VkPhysicalDevice);												// 找顯卡中 對應 Queue 的 Indices
	vector<char> __ReadShaderFile(const string&);															// 讀取 ShaderFile
	VkShaderModule __CreateShaderModule(const vector<char>&);												// 產生 Shader Module
	
	//////////////////////////////////////////////////////////////////////////
	// Swap Chain 的 Helper Function
	//////////////////////////////////////////////////////////////////////////
	SwapChainSupportDetails __QuerySwapChainSupport(VkPhysicalDevice);										// 檢查裝置是否支援 SwapChain
	VkSurfaceFormatKHR __ChooseSwapSurfaceFormat(const vector<VkSurfaceFormatKHR>&);						// 選擇裝置的 SurfaceFormat
	VkPresentModeKHR __ChooseSwapPresentMode(const vector<VkPresentModeKHR>&);								// 選擇裝置的 PresentMode
	VkExtent2D __ChooseSwapExtent(const VkSurfaceCapabilitiesKHR&);											// 根據裝置的 Capabillities 選擇 Extent

	// 檢查項目
	vector<const char*> deviceExtensionNames						= 
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

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

