#include "VulkanEngineApplication.h"

#pragma region VulkanMessage
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

	// 可以根據 Message Type 來濾掉一些東西
	// VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: Some event has happened that is unrelated to the specification or performance
	// VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: Something has happened that violates the specification or indicates a possible mistake
	// VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: Potential non-optimal use of Vulkan
	
    cerr << pCallbackData->pMessage << endl;

    return VK_FALSE;
}

// 由於這兩個都在 Extension 內
// 需要透過 Proxy 的方式去呼叫他們
static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}
static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}
#pragma endregion
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
#if defined(VKENGINE_DEBUG_DETAILS)
	cout << "glfwVulkanSupported: " << (glfwVulkanSupported() ? "True" : "False") << endl;
#endif
}
void VulkanEngineApplication::InitVulkan()
{
	__CreateVKInstance();
	__SetupDebugMessenger();
	__CreateSurface();
	__PickPhysicalDevice();
	__CreateLogicalDevice();
	__CreateSwapChain();
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
#if defined(VKENGINE_DEBUG_DETAILS)
	if (EnabledValidationLayer)
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
#endif
	vkDestroySwapchainKHR(device, swapChain, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
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
#if defined(VKENGINE_DEBUG_DETAILS)
	EnabledValidationLayer = __CheckValidationLayerSupport();
	if (!EnabledValidationLayer)
		cout << "ValidationLayer is requested, but it's not supported" << endl;
#endif

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

	// 取得 GLFW Extension
	uint32_t glfwExtensionCount										= 0;
	const char** glfwExtensions										= glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	vector<const char*> extNames(glfwExtensions, 					glfwExtensions + glfwExtensionCount);

	// 從 Vulkan 1.3.216 版本起
	// 會強制使用 VK_KHR_PORTABILITY_subset
	// 由於 Mac 是使用 MoltenVK
	// 目前不完全支援 Vulkan 的 VK_KHR_PORTABILITY_subset
	// https://vulkan.lunarg.com/doc/view/1.3.236.0/mac/getting_started.html#user-content-encountered-vk_error_incompatible_driver
#if defined(__APPLE__)
	extNames.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
	createInfo.flags												= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

// 設定 Debug Details
#if defined(VKENGINE_DEBUG_DETAILS)
	if (EnabledValidationLayer)
		extNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

	createInfo.enabledExtensionCount								= static_cast<uint32_t>(extNames.size());
	createInfo.ppEnabledExtensionNames								= extNames.data();

	// 這裡做兩件事情
	// 1. 設定 Global Layer (如果支援就先設定 Debug Layer)
	// 2. 測試支援那些 Vulkan Entension
#if defined(VKENGINE_DEBUG_DETAILS)
	// 1.
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (EnabledValidationLayer)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayersNames.size());
		createInfo.ppEnabledLayerNames = ValidationLayersNames.data();

       __PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = &debugCreateInfo;
	}
	else
		createInfo.enabledLayerCount = 0;


	// 2.
	uint32_t extensionCount;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
	std::cout << "available extensions:\n";

	for (const auto& extension : extensions)
		cout << '\t' << extension.extensionName << endl;
#endif

	// 建立 VKInstance
	VkResult result													= vkCreateInstance(&createInfo, nullptr, &instance);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create Vulkan Instance");
}
void VulkanEngineApplication::__SetupDebugMessenger()
{
#if !defined(VKENGINE_DEBUG_DETAILS)
	return;
#else
	if (EnabledValidationLayer)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
       	__PopulateDebugMessengerCreateInfo(createInfo);
		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
    		throw std::runtime_error("Failed to set up debug messenger");
	}
#endif
}
void VulkanEngineApplication::__CreateSurface()
{
	VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create window surface");
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
		if (__IsDeviceSuitable(device))
		{
			physiclaDevice = device;
			break;
		}
	
	if (physiclaDevice == VK_NULL_HANDLE)
		throw runtime_error("No Suitable GPUs");
}
void VulkanEngineApplication::__CreateLogicalDevice()
{
	QueueFamilyIndices indices										= __FindQueueFamilies(physiclaDevice);

	vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	set<uint32_t> uniqueQueueFamilies 								= {indices.GraphicsFamily.value(), indices.PresentFamily.value()};

	float queuePriority 											= 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
	{
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType 										= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex 							= queueFamily;
        queueCreateInfo.queueCount 									= 1;									// 預設來說這裡 Queue 只需要一個 (一般來說都是在 Multi-thread 中 create command buffers，然後在 Main Thread 一次全部送上去)
        queueCreateInfo.pQueuePriorities 							= &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

	// 對此裝置需要啟用哪些 Features
	VkPhysicalDeviceFeatures deviceFeatures{};

	// 真正建立 Logical Device
	VkDeviceCreateInfo createInfo{};
	createInfo.sType												= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos									= queueCreateInfos.data();
	createInfo.pEnabledFeatures										= &deviceFeatures;
	createInfo.queueCreateInfoCount									= 1;

#if defined(__APPLE__)
	// Mac 有開啟此功能，需要設定一下
	deviceExtensionNames.push_back("VK_KHR_portability_subset");
#endif
	createInfo.enabledExtensionCount								= static_cast<uint32_t>(deviceExtensionNames.size());
	createInfo.ppEnabledExtensionNames								= deviceExtensionNames.data();

#if defined(VKENGINE_DEBUG_DETAILS)
	if (EnabledValidationLayer)
	{
		createInfo.enabledLayerCount 								= static_cast<uint32_t>(ValidationLayersNames.size());
		createInfo.ppEnabledLayerNames								= ValidationLayersNames.data();
	}
#endif

	// 產生裝置完後，設定 Graphics Queue
	if (vkCreateDevice(physiclaDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
		throw runtime_error("Failed to create logical device");
    vkGetDeviceQueue(device, indices.GraphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.PresentFamily.value(), 0, &presentQueue);
}
void VulkanEngineApplication::__CreateSwapChain()
{
	SwapChainSupportDetails details 								= __QuerySwapChainSupport(physiclaDevice);

	VkSurfaceFormatKHR surfaceFormat 								= __ChooseSwapSurfaceFormat(details.Formats);
	VkPresentModeKHR presentMode 									= __ChooseSwapPresentMode(details.PresentModes);
	VkExtent2D extent												= __ChooseSwapExtent(details.Capbilities);

	// 需要多一張圖，預備下一個 Frame 繪製
	uint32_t imageCount 											= details.Capbilities.minImageCount + 1;
	if (details.Capbilities.maxImageCount > 0 && imageCount > details.Capbilities.maxImageCount)			// maxImageCount 可能為 0 (代表沒有最大值)，然後判斷是否超過最大值
		imageCount 													= details.Capbilities.maxImageCount;

	// 建立 SwapChain
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType												= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface												= surface;

	// 傳入上方的設定
	createInfo.imageColorSpace										= surfaceFormat.colorSpace;
	createInfo.imageFormat											= surfaceFormat.format;
	createInfo.presentMode											= presentMode;
	createInfo.imageExtent											= extent;
	createInfo.minImageCount										= imageCount;

	// 其他細節設定
	createInfo.imageArrayLayers										= 1;									// 如果要使用 Stereo 就會需要兩個 (兩個輸出)，不然一般都是一個
	createInfo.imageUsage											= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;	// 輸出的圖，可以做什麼使用的設定
	QueueFamilyIndices indices 										= __FindQueueFamilies(physiclaDevice);
	uint32_t queueFamilyIndices[]									= { indices.GraphicsFamily.value(), indices.PresentFamily.value() };
	
	// 判斷 Graphics & Present 是否在同一個 Queue
	if (indices.GraphicsFamily.value() != indices.PresentFamily.value())
	{
		// 不同 Queue 可以互相 share Images
		createInfo.imageSharingMode 								= VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount 							= 2;
		createInfo.pQueueFamilyIndices 								= queueFamilyIndices;
	}
	else
	{
		// 只能單一個 Queue 使用 Images，這個會效能會比較好
		createInfo.imageSharingMode 								= VK_SHARING_MODE_EXCLUSIVE;
		//createInfo.queueFamilyIndexCount 							= 0;
		//createInfo.pQueueFamilyIndices								= nullptr;
	}
	createInfo.preTransform											= details.Capbilities.currentTransform;	// 如果輸出的圖片有需要旋轉，需要更動這個參數
	createInfo.compositeAlpha										= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;	// 如果有需要 Blending With Other Windows，需調調整設定，不然預設都是忽略輸出的 Alpha （使用 Opaque_Bit_KHR）
	createInfo.clipped												= VK_TRUE;								// 會把看不見得 Pixel 忽略 (等同於 Unity Clip)
	createInfo.oldSwapchain											= VK_NULL_HANDLE;						// 當如果不支援的話，所做的動作是什麼

	// 建立 SwapChain
	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
		throw new runtime_error("Failed to create SwapChain");
}

//////////////////////////////////////////////////////////////////////////
// 比較 Minor 的 Helper Function
//////////////////////////////////////////////////////////////////////////
void VulkanEngineApplication::__PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &debugCreateInfo)
{
	debugCreateInfo.sType 											= VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCreateInfo.messageSeverity 								= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugCreateInfo.messageType 									= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugCreateInfo.pfnUserCallback 								= debugCallback;
	debugCreateInfo.pUserData 										= nullptr;
}
bool VulkanEngineApplication::__CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	vector<VkExtensionProperties> properties(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, properties.data());

	for(const auto& names : deviceExtensionNames)
		if (find_if(properties.begin(), properties.end(), [names](VkExtensionProperties p) { return strcmp(p.extensionName, names) == 0; }) == properties.end())							// 找到 end 也沒找到
			return false;
	return true;
}
bool VulkanEngineApplication::__IsDeviceSuitable(VkPhysicalDevice device)
{
	// 先檢查 Device Extension 是否都支援
	if (!__CheckDeviceExtensionSupport(device))
		return false;

	// 找出各種 Queue 是否支援
	QueueFamilyIndices indices 								= __FindQueueFamilies(device);
	if (!indices.IsCompleted())
		return false;	

	// 判斷 SwapChain 是否可以支援
	auto details											= __QuerySwapChainSupport(device);
	if (details.Formats.empty() || details.PresentModes.empty())
		return false;

	// 測試顯卡的一些細節
#if defined(VKENGINE_DEBUG_DETAILS)
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	cout << "Max Dimension of Texture Size: " << deviceProperties.limits.maxImageDimension2D << endl;
	cout << "Is Geometry Shader available: " << (deviceFeatures.geometryShader ? "True" : "False") << endl;	// Mac M1 不支援 (https://forum.unity.com/threads/geometry-shader-on-mac.1056659/)
#endif
	return true;
}
QueueFamilyIndices VulkanEngineApplication::__FindQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	// 檢查 QueueFamily 是否支援 Graphics 的 Queue
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);	
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
	for(int i = 0; i < queueFamilies.size(); i++)
	{
		// 判斷是否支援 Graphics 的 QueueFamily
		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkQueueFlagBits.html
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)											// 這一個 Queue Index 是處理 Graphics 相關的*/
			indices.GraphicsFamily = i;

		// 判斷 device 是否支援 presentation
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if (presentSupport)
			indices.PresentFamily = i;

		if (indices.IsCompleted())
			break;
	}
	return indices;																								// 無正常的可以處理 Graphics 的 Queue */
}


//////////////////////////////////////////////////////////////////////////
// 比較 Swap Chain 的 Function
//////////////////////////////////////////////////////////////////////////
SwapChainSupportDetails VulkanEngineApplication::__QuerySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.Capbilities);

	// Get Format
	uint32_t size = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &size, nullptr);
	if (size > 0)
	{
		details.Formats.resize(size);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &size, details.Formats.data());
	}

	// Get Present Mode
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &size, nullptr);
	if (size > 0)
	{
		details.PresentModes.resize(size);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &size, details.PresentModes.data());
	}
	return details;
}
VkSurfaceFormatKHR VulkanEngineApplication::__ChooseSwapSurfaceFormat(const vector<VkSurfaceFormatKHR>& availableFormats)
{
	for(const auto& format : availableFormats)
		if (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && format.format == VK_FORMAT_R8G8B8A8_SRGB)
			return format;
	return availableFormats[0];
}
VkPresentModeKHR VulkanEngineApplication::__ChooseSwapPresentMode(const vector<VkPresentModeKHR>& availablePresentModes)
{
	for(const auto& mode : availablePresentModes)
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR)					// 這個模式最比 FIFO 還省電，且不會造成 Tearing
			return mode;
	return VK_PRESENT_MODE_FIFO_KHR;
}
VkExtent2D VulkanEngineApplication::__ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	// 如果內部值是正常的
	if (capabilities.currentExtent.width != numeric_limits<uint32_t>::max())
		return capabilities.currentExtent;

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	VkExtent2D actualExtent =
	{
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height)
	};
	actualExtent.width = clamp(static_cast<uint32_t>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	actualExtent.height = clamp(static_cast<uint32_t>(height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
	return actualExtent;
}


#if defined(VKENGINE_DEBUG_DETAILS)
bool VulkanEngineApplication::__CheckValidationLayerSupport()
{
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	vector<VkLayerProperties> layerProperties(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());

	for (const char* layerName : ValidationLayersNames)
	{
		bool IsLayerFound = false;
		for(const auto currentProperties : layerProperties)
			if (strcmp(layerName, currentProperties.layerName) == 0)
			{
				IsLayerFound = true;
				break;
			}

		if (!IsLayerFound)
			return false;
	}
	return true;
}
#endif
#pragma endregion