#include "VulkanManager.h"

namespace VulkanEngine
{
	#pragma region Public
	VulkanManager::VulkanManager()
	{
		CreateVKInstance();
	}
	VulkanManager::~VulkanManager()
	{
		// 清掉 Vulkan 相關東西
#if defined(VKENGINE_DEBUG_DETAILS)
		if (EnabledValidationLayer)
			DestroyDebugUtilsMessengerEXT(Instance, DebugMessenger, nullptr);
#endif
		vkDestroyInstance(Instance, nullptr);
	}

	VkInstance VulkanManager::GetVKInstance()
	{
		return Instance;
	}
	#pragma endregion
	#pragma region Private
	void VulkanManager::CreateVKInstance()
	{
#if defined(VKENGINE_DEBUG_DETAILS)
		EnabledValidationLayer = __CheckValidationLayerSupport();
		if (!EnabledValidationLayer)
			cout << "ValidationLayer is requested, but it's not supported" << endl;
#endif

		// 建立 Application Info
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "VulkanEngine";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "DarkVulkanEngine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_3;

		// 設定到 Instance 中
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// 取得 GLFW Extension
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		vector<const char*> extNames(glfwExtensions, glfwExtensions + glfwExtensionCount);

		// 從 Vulkan 1.3.216 版本起
		// 會強制使用 VK_KHR_PORTABILITY_subset
		// 由於 Mac 是使用 MoltenVK
		// 目前不完全支援 Vulkan 的 VK_KHR_PORTABILITY_subset
		// https://vulkan.lunarg.com/doc/view/1.3.236.0/mac/getting_started.html#user-content-encountered-vk_error_incompatible_driver
#if defined(__APPLE__)
		extNames.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
		createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

		// 設定 Debug Details
#if defined(VKENGINE_DEBUG_DETAILS)
		if (EnabledValidationLayer)
			extNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

		createInfo.enabledExtensionCount = static_cast<uint32_t>(extNames.size());
		createInfo.ppEnabledExtensionNames = extNames.data();

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
		VkResult result = vkCreateInstance(&createInfo, nullptr, &Instance);
		if (result != VK_SUCCESS)
			throw runtime_error("Failed to create Vulkan Instance");
	}
	void VulkanManager::SetupDebugMessenger()
	{
#if defined(VKENGINE_DEBUG_DETAILS)
		if (EnabledValidationLayer)
		{
			VkDebugUtilsMessengerCreateInfoEXT createInfo{};
			__PopulateDebugMessengerCreateInfo(createInfo);
			if (CreateDebugUtilsMessengerEXT(Instance, &createInfo, nullptr, &DebugMessenger) != VK_SUCCESS)
				throw std::runtime_error("Failed to set up debug messenger");
		}
#endif
	}

	void VulkanManager::__PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo)
	{
		debugCreateInfo.sType 										= VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.messageSeverity 							= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType 								= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback 							= debugCallback;
		debugCreateInfo.pUserData 									= nullptr;
	}
	#pragma endregion
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
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}
	static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}
#pragma endregion
}