#pragma once
/*
* Vulkan 系統的 Application
*/
#include "Common/Common.h"
#include "Components/ImGuiWindowManager.h"
#include "Components/TextureManager.h"

#include "Components/Scene/SceneManager.h"

#include <vector>
#include <set>
#include <cstdint>
#include <limits>
#include <optional>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <chrono>

using namespace std;

// 測試 Vulkan 的一些細節使用
#define VKENGINE_DEBUG_DETAILS
#define USE_BARYCENTRIC_WIREFRAME																			// 畫 Wireframe 使用

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
	// Instance
	static VulkanEngineApplication* Instance;

	void Run();

	// Window Resized
	bool mFrameBufferResized = false;
	static void ResizeCallback(GLFWwindow*, int, int);

	// Vulkan Buffer Function
	void CreateBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkBuffer&, VkDeviceMemory&); // Create Buffer
	void CopyBuffer(VkBuffer, VkBuffer, VkDeviceSize);														// Copy Buffer

	// Vulkan Image Function
	void CreateImage(uint32_t, uint32_t, VkFormat, VkImageTiling, VkImageUsageFlags, VkMemoryPropertyFlags,	// Create Image
			VkImage&, VkDeviceMemory&);
	VkImageView CreateImageView(VkImage, VkFormat, VkImageAspectFlags);										// 輔助產生 ImageView

	// Get Vulkan Item
	static const uint32_t MAX_FRAME_IN_FLIGHTS						= 2;									// 最大上限的 Frame 數
	inline VkDevice GetDevice()										{ return mDevice; };					// 抓取 Device
	inline VkRenderPass GetRenderPass()								{ return mRenderPass; };				// 抓 RenderPass
	void GetViewportAndScissor(VkViewport&, VkRect2D&);                        								// 拿 Viewport & Scissor
	inline uint32_t GetCurrentFrameIndex()							{ return mCurrentFrameIndex; }			// 抓取目前在話那一個 Frame

private:
	void InitWindow();																						// 初始化視窗
	void InitVulkan();																						// 初始化 Vulkan
	void InitScene();																						// 初始化場景
	void MainLoop();																						// Main
	void Destroy();																							// 清空其他的資料
	
	void DrawFrame();																						// 繪製畫面
	void ReCreateSwapChain();																				// 重新建立 SwapChain

	// 視窗設定
	GLFWwindow* Window												= NULL;									// GLFW Window
	const int WIDTH													= 1600;									// 長
	const int HEIGHT												= 900;									// 寬

	// IMGUI
	ImGuiWindowManager* ImGuiWindowM;

	// Vulkan
	VkInstance mInstance;
	VkDebugUtilsMessengerEXT mDebugMessenger;
	VkSurfaceKHR mSurface;
	VkPhysicalDevice mPhysiclaDevice								= VK_NULL_HANDLE;
	VkDevice mDevice;
	
	// Vulkan Pipeline
	VkRenderPass mRenderPass;
	VkCommandPool mCommandPool;

	// Vulkan Queue
	QueueFamilyIndices mIndices;
	VkQueue mGraphicsQueue;
	VkQueue mPresentQueue;

	// Vulkan SwapChain
	VkSwapchainKHR SwapChain;
	vector<VkImage> SwapChainImages;
	vector<VkImageView> SwapChainImageViews;
	vector<VkFramebuffer> SwapChainFrameBuffers;
	VkFormat SwapChainImageFormat;
	VkExtent2D mSwapChainExtent;

	// Vulkan Depth Buffer
	VkImage mDepthImage;
	VkImageView mDepthImageView;
	VkDeviceMemory mDepthImageMemory;

	// ImGui DescriptorPool
	VkDescriptorPool mImGuiDescriptorPool;

	// Vulkan Command Buffer
	// 這裡是卡住上限，避免畫太多資料
	uint32_t mCurrentFrameIndex 									= 0;
	vector<VkCommandBuffer> mCommandBuffers;

	// Vulkan Semaphore & Fence
	// Semaphore: 主要是用來當作信號使用，等 Work 執行完後會觸發 Semaphore，會指使下一個 Work 繼續執行
	// Fence: 用來等待 Work 執行話之後，下方的程式碼才會繼續執行
	// https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Rendering_and_presentation#page_Semaphores
	vector<VkSemaphore> ImageAvailbleSemaphore;
	vector<VkSemaphore> RenderFinishedSemaphore;
	vector<VkFence> InFlightFences;

	// 貼圖相關
	//TextureManager* TextM;
	SceneManager* SceneM;

	//////////////////////////////////////////////////////////////////////////
	// Helper Init Function
	//////////////////////////////////////////////////////////////////////////
	void __CreateVKInstance();																				// Vulkan Instance
	void __SetupDebugMessenger();																			// 設定 Vulkan 的 Debug 工具
	void __CreateSurface();																					// 建立和視窗溝通的 Surface (GLFW & Vulkan)
	void __PickPhysicalDevice();																			// 選擇顯卡
	void __CreateLogicalDevice();																			// 根據對應的顯卡，去建立 Logical Device Interface
	void __CreateSwapChain();																				// 建立 Swap Chain
	void __CreateImageViews();																				// 建立 Image Views
	void __CreateRenderPass();																				// 建立 RenderPass，主要是要設定 Color Buffer & Depth Buffer
	void __CreateCommandPool();																				// 建立 Command Pool
	void __CreateDepthBuffers();																			// 產生 Depth 相關的資源
	void __CreateFrameBuffers();																			// 建立 Frame Buffer，把 SwapChain 的圖片畫上去
	void __CreateTextureImage();																			// 建立 Texture
	void __CreateIMGUIDescriptor();																			// 建立 IMGUI Descriptor Pool & Set
	void __CreateCommandBuffer();																			// 建立 Command Buffer
	void __CreateSyncObjects();

	//////////////////////////////////////////////////////////////////////////
	// Helper Render Function
	//////////////////////////////////////////////////////////////////////////
	void __SetupCommandBuffer(VkCommandBuffer, uint32_t);													// 將要執行的 Command 寫入 Command Buffer

	//////////////////////////////////////////////////////////////////////////
	// 比較 Minor 的 Helper Function
	//////////////////////////////////////////////////////////////////////////
	void __PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT&);							// 設定 Type
	bool __CheckDeviceExtensionSupport(VkPhysicalDevice);													// 檢查 DeviceExtension 是否支援某幾項
	bool __IsDeviceSuitable(VkPhysicalDevice);																// 是否為合適的顯卡
	QueueFamilyIndices __FindQueueFamilies(VkPhysicalDevice);												// 找顯卡中 對應 Queue 的 Indices
	uint32_t __FindMemoryType(uint32_t, VkMemoryPropertyFlags);												// 找到合適的 Memory Type

	//////////////////////////////////////////////////////////////////////////
	// Texture Format
	//////////////////////////////////////////////////////////////////////////
	VkFormat __GetDepthFormat();																			// 拿 Depth Texture Format
	VkFormat __FindSupportedTextureFormat(const vector<VkFormat>&, VkImageTiling, VkFormatFeatureFlags);	// 找到合適並支援的 Texture Format
	bool __HasStencilComponent(VkFormat);

	// 產生 Single Time Command
	VkCommandBuffer __BeginSingleTimeCommand();
	void __EndSingleTimeCommand(VkCommandBuffer);
	
	
	//////////////////////////////////////////////////////////////////////////
	// Swap Chain 的 Helper Function
	//////////////////////////////////////////////////////////////////////////
	SwapChainSupportDetails __QuerySwapChainSupport(VkPhysicalDevice);										// 檢查裝置是否支援 SwapChain
	VkSurfaceFormatKHR __ChooseSwapSurfaceFormat(const vector<VkSurfaceFormatKHR>&);						// 選擇裝置的 SurfaceFormat
	VkPresentModeKHR __ChooseSwapPresentMode(const vector<VkPresentModeKHR>&);								// 選擇裝置的 PresentMode
	VkExtent2D __ChooseSwapExtent(const VkSurfaceCapabilitiesKHR&);											// 根據裝置的 Capabillities 選擇 Extent
	void __CleanupSwapChain();																				// 清空 SwapChain
	void __ResetCameraAspect();																				// 重新設定 Camera Aspect

	// 檢查項目
	vector<const char*> deviceExtensionNames						= 
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

	// Barycentric
#if defined(USE_BARYCENTRIC_WIREFRAME)
	bool mEnableBarycentric											= false;								// 是否開啟這個功能
#endif

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