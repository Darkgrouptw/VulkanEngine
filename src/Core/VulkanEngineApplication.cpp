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
	__CreateVKInstance();
	__PickPhysicalDevice();
}
void VulkanEngineApplication::MainLoop()
{
	while (!glfwWindowShouldClose(window))																	// 接到是否關閉此視窗的 Flag
	{
		glfwPollEvents();																					// 抓出 GFLW 的事件 Queue
	}
}
void VulkanEngineApplication::Destroy()
{
	// 清掉 Vulkan Instance
	vkDestroyInstance(instance, nullptr);

	// 關閉 GLFW
	glfwDestroyWindow(window);
	glfwTerminate();
}

//////////////////////////////////////////////////////////////////////////
// Helper Function
//////////////////////////////////////////////////////////////////////////
void VulkanEngineApplication::__CreateVKInstance()
{
	// 建立 Application Info
	VkApplicationInfo appInfo{};
	appInfo.sType													= VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName										= "VulkanEngine";
	appInfo.applicationVersion										= VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName												= "DarkVulkanEngine";
	appInfo.engineVersion											= VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion												= VK_API_VERSION_1_3;

	// 設定到 Instance 中
	VkInstanceCreateInfo createInfo{};
	createInfo.sType												= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo										= &appInfo;

	
	// 由於 Vulakn 是底層的 API (和平台無關)
	// 必須從有一個和 GLFW 的系統溝通的 API
	// 撈 Entension
#if defined(__APPLE__)
	// 由於 Mac 是使用 MoltenVK
	// 目前不完全支援 Vulkan 的 VK_KHR_PORTABILITY_subset
	// https://vulkan.lunarg.com/doc/view/1.3.236.0/mac/getting_started.html#user-content-encountered-vk_error_incompatible_driver
	vector<const char*> extNames;
	extNames.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
	createInfo.flags												= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
	createInfo.enabledExtensionCount 								= static_cast<uint32_t>(extNames.size());
	createInfo.ppEnabledExtensionNames 								= extNames.data();
#else
	// 正常的模式下 取得 Extension
	uint32_t glfwExtensionCount										= 0;
	const char** glfwExtensions										= glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	createInfo.enabledExtensionCount								= glfwExtensionCount;
	createInfo.ppEnabledExtensionNames								= glfwExtensions;
#endif

	// 設定 Global Layer
	createInfo.enabledLayerCount = 0;

	// 測試支援那些 Vulkan Entension
	/*uint32_t extensionCount;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
	std::cout << "available extensions:\n";

	for (const auto& extension : extensions) {
		cout << '\t' << extension.extensionName << endl;
	}*/

	// 建立 VKInstance
	VkResult result													= vkCreateInstance(&createInfo, nullptr, &instance);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create Vulkan Instance");
}
void VulkanEngineApplication::__PickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);											// 抓出所有支援的 Device
	if (deviceCount == 0)
		throw runtime_error("Failed to find GPUs with Vulkan Support");

	// 並非所有的顯卡都符合設定
	// 所以這邊要做更進一步的 Check
	vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
	for(const auto& device: devices)
		if (__isDeviceSuitable(device))
		{
			physiclaDevice = device;
			break;
		}

	if (physiclaDevice == VK_NULL_HANDLE)
		throw runtime_error("No Suitable GPUs");
}
bool VulkanEngineApplication::__isDeviceSuitable(VkPhysicalDevice device)
{
	return true;
}
#pragma endregion
