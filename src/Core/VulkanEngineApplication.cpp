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
	__CreateLogicalDevice();
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

	// 清掉 Vulkan 相關東西
	vkDestroyDevice(device, nullptr);
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
	#ifdef VKENGINE_DEBUG_DETAILS
	uint32_t extensionCount;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
	std::cout << "available extensions:\n";

	for (const auto& extension : extensions) {
		cout << '\t' << extension.extensionName << endl;
	}
	#endif

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
	// 所以這邊要做更進一步的 Check (例如是否有 Geometry Shader 等)
	vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
	for(const auto& device: devices)
	{
		int queueIndex = __GetQueueIndexIfDeviceSuitable(device);
		if (queueIndex >= 0)
		{
			physiclaDevice = device;
			queueFamilyIndex = queueIndex;
			break;
		}
	}

	if (physiclaDevice == VK_NULL_HANDLE)
		throw runtime_error("No Suitable GPUs");
}
int VulkanEngineApplication::__GetQueueIndexIfDeviceSuitable(VkPhysicalDevice device)
{
	// 測試顯卡的一些細節
	#ifdef VKENGINE_DEBUG_DETAILS
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	cout << "Max Dimension of Texture Size: " << deviceProperties.limits.maxImageDimension2D << endl;
	cout << "Is Geometry Shader available: " << (deviceFeatures.geometryShader ? "True" : "False") << endl;	// Mac M1 不支援 (https://forum.unity.com/threads/geometry-shader-on-mac.1056659/)
	#endif

	// 檢查 QueueFamily 是否支援 Graphics 的 Queue
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);	
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
	for(int i = 0; i < queueFamilies.size(); i++)
	{
		const auto& queueFamily = queueFamilies[i];

		// 顯示到底有哪些東西
		#ifdef VKENGINE_DEBUG_DETAILS
		cout << device << " " << queueFamily.queueFlags << endl;
		#endif

		// 判斷是否支援 Graphics 的 QueueFamily
		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkQueueFlagBits.html
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			cout << "Graphics Queue: " << i << endl;
			return i;																						// 這一個 Queue Index 是處理 Graphics 相關的
		}
	}
	return -1;																								// 無正常的可以處理 Graphics 的 Queue 
}
void VulkanEngineApplication::__CreateLogicalDevice()
{
	// 建立 QueueInfo
	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType 											= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex 								= queueFamilyIndex;

	float priorties 												= 1.0f;
	queueCreateInfo.pQueuePriorities								= &priorties;							// 0 ~ 1 

	// 預設來說這裡 Queue 只需要一個 (一般來說都是在 Multi-thread 中 create command buffers，然後在 Main Thread 一次全部送上去)
	queueCreateInfo.queueCount 										= 1;

	// 對此裝置需要啟用哪些 Features
	VkPhysicalDeviceFeatures deviceFeatures{};

	// 真正建立 Logical Device
	VkDeviceCreateInfo createInfo{};
	createInfo.sType												= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos									= &queueCreateInfo;
	createInfo.pEnabledFeatures										= &deviceFeatures;
	createInfo.queueCreateInfoCount									= 1;

	// 暫時不需要
	createInfo.enabledExtensionCount								= 0;
	createInfo.enabledLayerCount									= 0;

	// 產生裝置完後，設定 Graphics Queue
	if (vkCreateDevice(physiclaDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
		throw runtime_error("Failed to create logical device");
	vkGetDeviceQueue(device, queueFamilyIndex, 0, &graphicsQueue);
}
#pragma endregion
