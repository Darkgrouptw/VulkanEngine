#include "Core/VulkanEngineApplication.h"

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

	Window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
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
	__CreateImageViews();
	__CreateRenderPass();
	__CreateGraphicsPipeline();
	__CreateFrameBuffer();
	__CreateCommandPool();
	__CreateVertexBuffer();
	__CreateCommandBuffer();
	__CreateSyncObjects();
}
void VulkanEngineApplication::MainLoop()
{
	while (!glfwWindowShouldClose(Window))																	// 接到是否關閉此視窗的 Flag
	{
		glfwPollEvents();																					// 抓出 GFLW 的事件 Queue
		DrawFrame();
	}
	vkDeviceWaitIdle(Device);
}
void VulkanEngineApplication::Destroy()
{
	#pragma region SwapChain
	__CleanupSwapChain();
	#pragma endregion
	#pragma region SyncObjects
	for (int i = 0; i < MAX_FRAME_IN_FLIGHTS; i++)
	{
		vkDestroySemaphore(Device, ImageAvailbleSemaphore[i], nullptr);
		vkDestroySemaphore(Device, RenderFinishedSemaphore[i], nullptr);
		vkDestroyFence(Device, InFlightFences[i], nullptr);
	}
	#pragma endregion
	#pragma region VertexBuffer
	vkDestroyBuffer(Device, VertexBuffer, nullptr);
	vkFreeMemory(Device, VertexBufferMemory, nullptr);
	#pragma endregion
	#pragma region Command Burffer
	// 不用 Destroy
	#pragma endregion
	#pragma region Command Pool
	vkDestroyCommandPool(Device, CommandPool, nullptr);
	#pragma endregion
	#pragma region Graphics Pipeline
	vkDestroyPipeline(Device, GraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(Device, PipelineLayout, nullptr);
	#pragma endregion
	#pragma region Render Pass
	vkDestroyRenderPass(Device, RenderPass, nullptr);
	#pragma endregion
	#pragma region Device
	vkDestroyDevice(Device, nullptr);
	#pragma endregion
	#pragma region Surface
	vkDestroySurfaceKHR(Instance, Surface, nullptr);
	#pragma endregion
	#pragma region Debug Messager
	// 清掉 Vulkan 相關東西
#if defined(VKENGINE_DEBUG_DETAILS)
	if (EnabledValidationLayer)
		DestroyDebugUtilsMessengerEXT(Instance, DebugMessenger, nullptr);
#endif
	#pragma endregion
	#pragma region Instance
	vkDestroyInstance(Instance, nullptr);
	#pragma endregion

	// 關閉 GLFW
	glfwDestroyWindow(Window);
	glfwTerminate();
}

void VulkanEngineApplication::DrawFrame()
{
	// 這你需要等待幾個步驟
	// 1. 等待前一幀的資料繪製完成
	// 2. 取得 Swap Chain 的圖片
	// 3. 設定 Commands 到 Command Buffer 中來繪製整個場景到 Image 中
	// 4. Submit Command Buffer
	// 5. 顯示 Swap Chain Image
	#pragma region 1.
	vkWaitForFences(Device, 1, &InFlightFences[CurrentFrameIndex], VK_TRUE, UINT64_MAX);
	#pragma endregion
	#pragma region 2.
	// 這裡要判斷是否已經過期
	// 有兩種狀況
	// 1. swap chain 的資料過期了 （通常發生在視窗大小改變，要重新建立新的 swap chain） 
	// 2. 可以繼續表現到 surface 上
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(Device, SwapChain, UINT64_MAX, ImageAvailbleSemaphore[CurrentFrameIndex], VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		ReCreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) // VK_SUBOPTIMAL_KHR 代表，雖然可以正常顯示到 surface 上，但是 surface 其實不需要了
		throw runtime_error("Failed to acquire swap chain image");

	vkResetFences(Device, 1, &InFlightFences[CurrentFrameIndex]);											// Reset Fences
	#pragma endregion
	#pragma region 3.
	// Reset & Write
	vkResetCommandBuffer(CommandBuffers[CurrentFrameIndex], 0);												// 後面的參數 Flag，目前還不用，暫時先留 0
	__SetupCommandBuffer(CommandBuffers[CurrentFrameIndex], imageIndex);
	#pragma endregion
	#pragma region 4.
	VkSubmitInfo submitInfo{};
	submitInfo.sType												= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	
	// 等待 Semaphore 完成，在做 pass
	VkSemaphore waitSemaphores[] 									= { ImageAvailbleSemaphore[CurrentFrameIndex] };
	VkPipelineStageFlags waitStages[]								= { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount									= static_cast<uint32_t>(sizeof(waitSemaphores) / sizeof(VkSemaphore));
	submitInfo.pWaitSemaphores										= waitSemaphores;
	submitInfo.pWaitDstStageMask									= waitStages;

	submitInfo.commandBufferCount									= 1;
	submitInfo.pCommandBuffers										= &CommandBuffers[CurrentFrameIndex];

	// 完成此 Submit 要觸發 singalSempahore
	VkSemaphore signalSemphores[]									= { RenderFinishedSemaphore[CurrentFrameIndex] };
	submitInfo.signalSemaphoreCount									= static_cast<uint32_t>(sizeof(signalSemphores) / sizeof(VkSemaphore));
	submitInfo.pSignalSemaphores									= signalSemphores;

	if (vkQueueSubmit(GraphicsQueue, 1, &submitInfo, InFlightFences[CurrentFrameIndex]) != VK_SUCCESS)
		throw runtime_error("Failed to submit draw command buffer");
	#pragma endregion
	#pragma region 5.
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType												= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount									= 1;
	presentInfo.pWaitSemaphores										= signalSemphores;

	VkSwapchainKHR swapChains[] 									= { SwapChain };
	presentInfo.swapchainCount										= static_cast<uint32_t>(sizeof(swapChains) / sizeof(VkSwapchainKHR));
	presentInfo.pSwapchains											= swapChains;
	presentInfo.pImageIndices										= &imageIndex;
	
	vkQueuePresentKHR(GraphicsQueue, &presentInfo);

	// 切換下一張
	CurrentFrameIndex = (CurrentFrameIndex + 1) % MAX_FRAME_IN_FLIGHTS;
	#pragma endregion
}
void VulkanEngineApplication::ReCreateSwapChain()
{
	// 等待動作都 Idle 完之後，再繼續做清除的動作
	vkDeviceWaitIdle(Device);

	__CleanupSwapChain();
	__CreateSwapChain();
	__CreateImageViews();
	__CreateFrameBuffer();
}

//////////////////////////////////////////////////////////////////////////
// Helper Init Function
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
	VkResult result													= vkCreateInstance(&createInfo, nullptr, &Instance);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create Vulkan Instance");
}
void VulkanEngineApplication::__SetupDebugMessenger()
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
void VulkanEngineApplication::__CreateSurface()
{
	VkResult result = glfwCreateWindowSurface(Instance, Window, nullptr, &Surface);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create window surface");
}
void VulkanEngineApplication::__PickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(Instance, &deviceCount, nullptr);											// 抓出所有支援的 Device
	if (deviceCount == 0)
		throw runtime_error("Failed to find GPUs with Vulkan Support");

	// 並非所有的顯卡都符合設定
	// 所以這邊要做更進一步的 Check (例如是否有 Geometry Shader 等)
	vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(Instance, &deviceCount, devices.data());
	for(const auto& device: devices)
		if (__IsDeviceSuitable(device))
		{
			PhysiclaDevice = device;
			break;
		}
	
	if (PhysiclaDevice == VK_NULL_HANDLE)
		throw runtime_error("No Suitable GPUs");
}
void VulkanEngineApplication::__CreateLogicalDevice()
{
	QueueFamilyIndices indices										= __FindQueueFamilies(PhysiclaDevice);

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
	if (vkCreateDevice(PhysiclaDevice, &createInfo, nullptr, &Device) != VK_SUCCESS)
		throw runtime_error("Failed to create logical device");
    vkGetDeviceQueue(Device, indices.GraphicsFamily.value(), 0, &GraphicsQueue);
    vkGetDeviceQueue(Device, indices.PresentFamily.value(), 0, &PresentQueue);
}
void VulkanEngineApplication::__CreateSwapChain()
{
	SwapChainSupportDetails details 								= __QuerySwapChainSupport(PhysiclaDevice);
	VkSurfaceFormatKHR surfaceFormat 								= __ChooseSwapSurfaceFormat(details.Formats);
	VkPresentModeKHR presentMode 									= __ChooseSwapPresentMode(details.PresentModes);
	VkExtent2D extent												= __ChooseSwapExtent(details.Capbilities);

#if defined(VKENGINE_DEBUG_DETAILS)
	cout << "Framebuffer: " << extent.width << " x " << extent.height << endl;
#endif

	// 需要多一張圖，預備下一個 Frame 繪製
	uint32_t imageCount 											= details.Capbilities.minImageCount + 1;
	if (details.Capbilities.maxImageCount > 0 && imageCount > details.Capbilities.maxImageCount)			// maxImageCount 可能為 0 (代表沒有最大值)，然後判斷是否超過最大值
		imageCount 													= details.Capbilities.maxImageCount;

	// 建立 SwapChain
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType												= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface												= Surface;

	// 傳入上方的設定
	createInfo.imageColorSpace										= surfaceFormat.colorSpace;
	createInfo.imageFormat											= surfaceFormat.format;
	createInfo.presentMode											= presentMode;
	createInfo.imageExtent											= extent;
	createInfo.minImageCount										= imageCount;

	// 其他細節設定
	createInfo.imageArrayLayers										= 1;									// 如果要使用 Stereo 就會需要兩個 (兩個輸出)，不然一般都是一個
	createInfo.imageUsage											= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;	// 輸出的圖，可以做什麼使用的設定
	QueueFamilyIndices indices 										= __FindQueueFamilies(PhysiclaDevice);
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
	if (vkCreateSwapchainKHR(Device, &createInfo, nullptr, &SwapChain) != VK_SUCCESS)
		throw new runtime_error("Failed to create SwapChain");
	
	vkGetSwapchainImagesKHR(Device, SwapChain, &imageCount, nullptr);
	SwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(Device, SwapChain, &imageCount, SwapChainImages.data());

	SwapChainImageFormat 											= surfaceFormat.format;
	SwapChainExtent													= extent;
}
void VulkanEngineApplication::__CreateImageViews()
{
	SwapChainImageViews.resize(SwapChainImages.size());
	for (size_t i = 0; i < SwapChainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType 											= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image											= SwapChainImages[i];
		createInfo.viewType											= VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format											= SwapChainImageFormat;

		// Image 的 Range 設定 0 ~ 1
		createInfo.components.r										= VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g										= VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b										= VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a										= VK_COMPONENT_SWIZZLE_IDENTITY;

		// 其他用途的設定 (Mipmap 等)
		createInfo.subresourceRange.aspectMask						= VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		// Create Image
		if (vkCreateImageView(Device, &createInfo, nullptr, &SwapChainImageViews[i]) != VK_SUCCESS)
			throw runtime_error("Failed to create ImageView");
	}
}
void VulkanEngineApplication::__CreateRenderPass()
{
	// 設定 Color Buffer
	VkAttachmentDescription colorBufferAttachment{};
	colorBufferAttachment.format									= SwapChainImageFormat;
	colorBufferAttachment.samples									= VK_SAMPLE_COUNT_1_BIT;
	colorBufferAttachment.loadOp									= VK_ATTACHMENT_LOAD_OP_CLEAR;			// 在 Load 完之後，先做 Clear
	colorBufferAttachment.storeOp									= VK_ATTACHMENT_STORE_OP_STORE;			// Render 完之後，會存在 memory 內，方便以後讀取

	// 暫時不會用到
	// 可以設定成 DONT_CARE
	colorBufferAttachment.stencilLoadOp								= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorBufferAttachment.stencilStoreOp							= VK_ATTACHMENT_STORE_OP_DONT_CARE;

	colorBufferAttachment.initialLayout								= VK_IMAGE_LAYOUT_UNDEFINED;			// 在 layout 之前，不需要設定
	colorBufferAttachment.finalLayout								= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;		// 做 SwapChain

	// Attachment & Subpass
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment									= 0;									// 由於 Color Attach Index 為 0
	colorAttachmentRef.layout										= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // 希望得到 Best performance Of Color Attachment

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint										= VK_PIPELINE_BIND_POINT_GRAPHICS;		// 此 Pipeline 是用來 Graphics 的 (或是可以切換成 Compute)
	subpass.colorAttachmentCount									= 1;
	subpass.pColorAttachments										= &colorAttachmentRef;

	// Subpass 也需要等待 ImageAvailableSemaphore
	// 需要設定 Dependency
	VkSubpassDependency dependency{};
	dependency.srcSubpass											= VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass											= 0;
	dependency.srcStageMask											= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // 確定 RenderPass 會等待 Subpass 完成
	dependency.dstStageMask											= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask										= 0;
	dependency.dstAccessMask										= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	// Create Render Pass
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType											= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount									= 1;
	renderPassInfo.pAttachments										= &colorBufferAttachment;
	renderPassInfo.subpassCount										= 1;
	renderPassInfo.pSubpasses										= &subpass;
	renderPassInfo.dependencyCount									= 1;
	renderPassInfo.pDependencies									= &dependency;

	if (vkCreateRenderPass(Device, &renderPassInfo, nullptr, &RenderPass) != VK_SUCCESS)
		throw runtime_error("Failed to create render pass");
}
void VulkanEngineApplication::__CreateGraphicsPipeline()
{
	// 讀取檔案並建立 Shader Module
	#pragma region VkShaderModule
	// 檢查是否存在 build 資料夾內
	// 1. 如果是 不須增加 "./build/"
	// 2. 如果不是 增加 "./build/"
	string currentPath												= filesystem::current_path().string();
	string buildPath												= "build";
	bool IsInBuildDir												= false;
	if (currentPath.length() >= buildPath.length())
		IsInBuildDir = currentPath.compare(currentPath.length() - buildPath.length(), buildPath.length(), buildPath) == 0; // 如果結尾是 build 輸出 0
	vector<char> vertexShader, fragmentShader;
	if (IsInBuildDir)
	{
		vertexShader 												= __ReadShaderFile("Shaders/Test.vert.spv");
		fragmentShader												= __ReadShaderFile("Shaders/Test.frag.spv");
	}
	else
	{
		vertexShader 												= __ReadShaderFile("./build/Shaders/Test.vert.spv");
		fragmentShader												= __ReadShaderFile("./build/Shaders/Test.frag.spv");
	}

	VkShaderModule vertexModule										= __CreateShaderModule(vertexShader);
	VkShaderModule fragmentModule									= __CreateShaderModule(fragmentShader);
	
	// 產生 Graphics Pipeline
	const char* mainFunctionName 									= "main"; 								// 入口的 Function 名稱
	VkPipelineShaderStageCreateInfo vertexStageCreateInfo{};
	vertexStageCreateInfo.sType										= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexStageCreateInfo.stage										= VK_SHADER_STAGE_VERTEX_BIT;
	vertexStageCreateInfo.module									= vertexModule;
	vertexStageCreateInfo.pName										= mainFunctionName;

	VkPipelineShaderStageCreateInfo fragmentStageCreateInfo{};
	fragmentStageCreateInfo.sType									= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentStageCreateInfo.stage									= VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentStageCreateInfo.module									= fragmentModule;
	fragmentStageCreateInfo.pName									= mainFunctionName;

	// 創建 Shader Stage
	VkPipelineShaderStageCreateInfo shaderStages[] = {vertexStageCreateInfo, fragmentStageCreateInfo};
	#pragma endregion
	#pragma region Graphics Pipeline Layout
	// Graphics Pipeline Layout:
	// 1. Vertex Input												點的資料
	// 2. Input Assembler 											點怎麼連接
	// 3. Vertex Shader	(From Code)											
	// 4. Rasterization & MultiSamping
	// 5. Fragment Shader (Fom Code)
	// 6. Color Blending
	// 7. FrameBuffer
	#pragma region 1. Vertex Input
	auto attribeDesc												= VertexBufferInfo::GetAttributeDescription();
	auto bindingDesc												= VertexBufferInfo::GetBindingDescription();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType											= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexAttributeDescriptionCount					= static_cast<uint32_t>(attribeDesc.size());
	vertexInputInfo.pVertexAttributeDescriptions					= attribeDesc.data();
	vertexInputInfo.vertexBindingDescriptionCount					= 1;
	vertexInputInfo.pVertexBindingDescriptions						= &bindingDesc;
	#pragma endregion
	#pragma region 2. Input Assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
	inputAssemblyInfo.sType											= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyInfo.topology										= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;	// 每三個點使用後，不複用
	inputAssemblyInfo.primitiveRestartEnable						= VK_FALSE;								// 上方為 _STRIP 才會使用
	#pragma endregion
	#pragma region Viewport & Scissor
	// Generate Viewport & Scissor
	VkViewport viewport{};
	VkRect2D scissor{};
	__GenerateInitViewportAndScissor(viewport, scissor);

	// Viewport
	VkPipelineViewportStateCreateInfo viewportInfo{};
	viewportInfo.sType												= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportInfo.viewportCount										= 1;
	viewportInfo.scissorCount										= 1;
	viewportInfo.pViewports											= &viewport;
	viewportInfo.pScissors											= &scissor;
	#pragma endregion
	#pragma region 4. Rasterization & MultiSamping
	VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
	rasterizationInfo.sType											= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationInfo.depthClampEnable								= VK_FALSE;								// 對於 Shadow Map 可能會有需要 Near 到 Far 之間的資訊，其他都不需要設為 True
	rasterizationInfo.rasterizerDiscardEnable						= VK_FALSE;								// 如果設定成 True，會造成 geometry 不會傳入 rasterization
	rasterizationInfo.cullMode										= VK_CULL_MODE_BACK_BIT;				// 去除 Back face
	rasterizationInfo.frontFace										= VK_FRONT_FACE_CLOCKWISE;				// 逆時針的 Vertex，算 Front Face
	rasterizationInfo.depthBiasEnable								= VK_FALSE;
	rasterizationInfo.lineWidth										= 1.0;
	// 當設定為 False，底下設定就不需要設定
	//rasterizationInfo.depthBiasConstantFactor 						= 0.0f;
	//rasterizationInfo.depthBiasClamp 								= 0.0f;
	//rasterizationInfo.depthBiasSlopeFactor 							= 0.0f;

	// Multisampling (Anti-Aliasing)
	VkPipelineMultisampleStateCreateInfo multisamplingInfo{};
	multisamplingInfo.sType											= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingInfo.rasterizationSamples							= VK_SAMPLE_COUNT_1_BIT;
	multisamplingInfo.sampleShadingEnable							= VK_FALSE;
	// 上方要設定為 True，下方才要設定
	//multisamplingInfo.minSampleShading 								= 1.0f;
	//multisamplingInfo.pSampleMask 									= nullptr;
	//multisamplingInfo.alphaToCoverageEnable							= VK_FALSE;
	//multisamplingInfo.alphaToOneEnable 								= VK_FALSE;
	#pragma endregion
	#pragma region 6. Color Blending
	// 這裡要設定兩個
	// 1. 設定 Color Blending 後的 FrameBuffer 上的設定
	// 2. 設定 Global 的 Color Blending
	
	// 1.
	VkPipelineColorBlendAttachmentState colorBlendAttachmentInfo{};
	colorBlendAttachmentInfo.colorWriteMask							= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachmentInfo.blendEnable							= VK_FALSE;
	// 開啟 Blend 才需要設定
	//colorBlendAttachmentInfo.srcColorBlendFactor 					= VK_BLEND_FACTOR_ONE; // Optional
	//colorBlendAttachmentInfo.dstColorBlendFactor 					= VK_BLEND_FACTOR_ZERO; // Optional
	//colorBlendAttachmentInfo.colorBlendOp 							= VK_BLEND_OP_ADD; // Optional
	//colorBlendAttachmentInfo.srcAlphaBlendFactor 					= VK_BLEND_FACTOR_ONE; // Optional
	//colorBlendAttachmentInfo.dstAlphaBlendFactor 					= VK_BLEND_FACTOR_ZERO; // Optional
	//colorBlendAttachmentInfo.alphaBlendOp 							= VK_BLEND_OP_ADD; // Optional	

	// 2.
	VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
	colorBlendInfo.sType											= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendInfo.logicOpEnable									= VK_FALSE;
	colorBlendInfo.attachmentCount									= 1;
	colorBlendInfo.pAttachments										= &colorBlendAttachmentInfo;
	#pragma endregion

	// 而在 OpenGL 中已經設定好 Fixed Function
	// 在 Vulkan 中要手動設定
	vector<VkDynamicState> states = {
		VK_DYNAMIC_STATE_VIEWPORT,									// 需要手動設定 vkCmdSetViewport
		VK_DYNAMIC_STATE_SCISSOR									// 需要手動設定 vkCmdSetScissor
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType												= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount									= static_cast<uint32_t>(states.size());
	dynamicState.pDynamicStates										= states.data();

	// 設定 Layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType										= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount								= 0;
	pipelineLayoutInfo.pSetLayouts									= nullptr;
	pipelineLayoutInfo.pushConstantRangeCount						= 0;
	pipelineLayoutInfo.pPushConstantRanges							= nullptr;

	if (vkCreatePipelineLayout(Device, &pipelineLayoutInfo, nullptr, &PipelineLayout) != VK_SUCCESS)
		throw runtime_error("Failed to create pipeline");
	#pragma endregion
	#pragma region Graphics Pipeline
	// 建立 Graphics Pipeline
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType												= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount											= static_cast<uint32_t>(sizeof(shaderStages) / sizeof(VkPipelineShaderStageCreateInfo));
	pipelineInfo.pStages											= shaderStages;

	pipelineInfo.pVertexInputState									= &vertexInputInfo;
	pipelineInfo.pInputAssemblyState								= &inputAssemblyInfo;
	pipelineInfo.pViewportState										= &viewportInfo;
	pipelineInfo.pRasterizationState								= &rasterizationInfo;
	pipelineInfo.pMultisampleState									= &multisamplingInfo;
	pipelineInfo.pDepthStencilState									= nullptr;
	pipelineInfo.pColorBlendState									= &colorBlendInfo;
	pipelineInfo.pDynamicState										= &dynamicState;

	pipelineInfo.layout												= PipelineLayout;
	pipelineInfo.renderPass											= RenderPass;
	pipelineInfo.subpass											= 0;
	pipelineInfo.basePipelineHandle									= VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &GraphicsPipeline) != VK_SUCCESS)
		throw runtime_error("Failed to create Graphics Pipeline");
	#pragma endregion
	#pragma region Destroy Module
	vkDestroyShaderModule(Device, vertexModule, nullptr);
	vkDestroyShaderModule(Device, fragmentModule, nullptr);
	#pragma endregion
}
void VulkanEngineApplication::__CreateFrameBuffer()
{
	SwapChainFrameBuffers.resize(SwapChainImageViews.size());
	for (size_t i = 0; i < SwapChainImageViews.size(); i++)
	{
		VkImageView attachments[] = { SwapChainImageViews[i] };
		VkFramebufferCreateInfo frameBufferInfo{};
		frameBufferInfo.sType										= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferInfo.renderPass									= RenderPass;
		frameBufferInfo.attachmentCount								= static_cast<uint32_t>(sizeof(attachments) / sizeof(VkImageView));
		frameBufferInfo.pAttachments								= attachments;
		frameBufferInfo.width										= SwapChainExtent.width;
		frameBufferInfo.height										= SwapChainExtent.height;
		frameBufferInfo.layers										= 1;

		if (vkCreateFramebuffer(Device, &frameBufferInfo, nullptr, &SwapChainFrameBuffers[i]) != VK_SUCCESS)
			throw runtime_error("Failed to create framebuffer");
	}
}
void VulkanEngineApplication::__CreateCommandPool()
{
	QueueFamilyIndices indices 										= __FindQueueFamilies(PhysiclaDevice);
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType													= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags													= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex										= indices.GraphicsFamily.value();

	if (vkCreateCommandPool(Device, &poolInfo, nullptr, &CommandPool) != VK_SUCCESS)
		throw runtime_error("Failed to create command pool");
}
void VulkanEngineApplication::__CreateVertexBuffer()
{
	VkDeviceSize bufferSize											= sizeof(VertexBufferInfo) * vertices.size();

	__CreateBuffer(
		bufferSize,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,							// 需要開啟這兩個 tag 才可以從 CPU 送上資料到 GPU
		VertexBuffer,
		VertexBufferMemory
		);

	void* data;
	vkMapMemory(Device, VertexBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(Device, VertexBufferMemory);
}
void VulkanEngineApplication::__CreateCommandBuffer()
{
	CommandBuffers.resize(MAX_FRAME_IN_FLIGHTS);
	
	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType												= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool										= CommandPool;
	allocateInfo.level												= VK_COMMAND_BUFFER_LEVEL_PRIMARY;		// 如果是 Primary，代表直接送 Command Buffer，無法被其他 Command Buffer 讀取
	allocateInfo.commandBufferCount									= static_cast<uint32_t>(CommandBuffers.size());

	if (vkAllocateCommandBuffers(Device, &allocateInfo, CommandBuffers.data()) != VK_SUCCESS)
		throw runtime_error("Failed to create command buffer");
}
void VulkanEngineApplication::__CreateSyncObjects()
{
	ImageAvailbleSemaphore.resize(MAX_FRAME_IN_FLIGHTS);
	RenderFinishedSemaphore.resize(MAX_FRAME_IN_FLIGHTS);
	InFlightFences.resize(MAX_FRAME_IN_FLIGHTS);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType												= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType													= VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags													= VK_FENCE_CREATE_SIGNALED_BIT;			// Create 完後，在第一次 DrawFrame 的時候就不會卡住

	for (int i = 0; i < MAX_FRAME_IN_FLIGHTS; i++)
	{
		if (vkCreateSemaphore(Device, &semaphoreInfo, nullptr, &ImageAvailbleSemaphore[i]) 					!= VK_SUCCESS ||
			vkCreateSemaphore(Device, &semaphoreInfo, nullptr, &RenderFinishedSemaphore[i]) 				!= VK_SUCCESS ||
			vkCreateFence(Device, &fenceInfo, nullptr, &InFlightFences[i])									!= VK_SUCCESS)
			throw runtime_error("Failed to create sync objects");
	}
}

//////////////////////////////////////////////////////////////////////////
// Helper Render Function
//////////////////////////////////////////////////////////////////////////
void VulkanEngineApplication::__GenerateInitViewportAndScissor(VkViewport& viewport, VkRect2D& scissor)
{
	// Viewport
	viewport.x 														= 0;
	viewport.y 														= 0;
	viewport.width 													= SwapChainExtent.width;
	viewport.height													= SwapChainExtent.height;
	viewport.minDepth												= 0;
	viewport.maxDepth												= 1;									// 設定 Depth 0 ~ 1

	// Scissor
	// https://vulkan-tutorial.com/images/viewports_scissors.png
	scissor.offset													= {0, 0};
	scissor.extent													= SwapChainExtent;
}
void VulkanEngineApplication::__SetupCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType													= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		throw runtime_error("Failed to begin recording command buffer");

	// 設定 Render Pass
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType											= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass										= RenderPass;
	renderPassInfo.framebuffer										= SwapChainFrameBuffers[imageIndex];
	renderPassInfo.renderArea.offset								= { 0, 0 };
	renderPassInfo.renderArea.extent								= SwapChainExtent;

	// Clear
	VkClearValue clearColor 										= {{{ 0.0f, 0.0f, 0.0f, 1.0f }}};
	renderPassInfo.clearValueCount									= 1;
	renderPassInfo.pClearValues										= &clearColor;

	// 設定 RenderPass (且無其他 Pass => CONTENTS_INLINE)
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline);
		
		// 由於前面設定 VkPipelineDynamicStateCreateInfo
		// 設定了 VK_DYNAMIC_STATE_VIEWPORT & VK_DYNAMIC_STATE_SCISSOR
		// 所以這裡需要在指定一次
		VkViewport viewport{};
		VkRect2D scissor{};
		__GenerateInitViewportAndScissor(viewport, scissor);

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		// 送 Buffer 上去 
		VkBuffer buffers[]											= { VertexBuffer };
		VkDeviceSize offsets[]										= { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
		vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);

	}
	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		throw runtime_error("Failed to record command buffer");
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
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, Surface, &presentSupport);
		if (presentSupport)
			indices.PresentFamily = i;

		if (indices.IsCompleted())
			break;
	}
	return indices;																								// 無正常的可以處理 Graphics 的 Queue */
}
vector<char> VulkanEngineApplication::__ReadShaderFile(const string& path)
{
	// 讀取檔案
	// ate: 從檔案的結果開始讀（主要是判斷檔案的大小，之後還可以移動此指標到想要的位置）
	// binary: 避免 text transformations
	ifstream file(path, ios::ate | ios::binary);
	if (!file.is_open())
        throw runtime_error("Failed to open the file(" + path + ")");

	size_t fileSize 												= file.tellg();
	vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();
	return buffer;
}
VkShaderModule VulkanEngineApplication::__CreateShaderModule(const vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType												= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize												= code.size();
	createInfo.pCode												= reinterpret_cast<const uint32_t*>(code.data());	// 要將原本的 char* 轉成 uint32_t*

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		throw runtime_error("Failed to create shader module");
	return shaderModule;
}
uint32_t VulkanEngineApplication::__FindMemoryType(uint32_t typeFiler, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(PhysiclaDevice, &memProperties);
#if defined(VKENGINE_DEBUG_DETAILS)
	cout << "Memory Type Bits: " << typeFiler << endl;
	cout << "Memory Type Count: " << memProperties.memoryTypeCount << endl;
#endif
	for(uint32_t i = 0 ; i < memProperties.memoryTypeCount; i++)
		if (typeFiler & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	throw runtime_error("Failed to find suitable memory type");
}
void VulkanEngineApplication::__CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType												= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size													= size;
	bufferInfo.usage												= usage;
	bufferInfo.sharingMode											= VK_SHARING_MODE_EXCLUSIVE;			// 只有在 Graphics Queue 會用到，暫時先給 Exclusive

	if (vkCreateBuffer(Device, &bufferInfo, nullptr, &VertexBuffer) != VK_SUCCESS)
		throw std::runtime_error("Failed to create vertex buffer");

	// 設定 Buffer 內的大小
	VkMemoryRequirements memRequirements{};
	vkGetBufferMemoryRequirements(Device, VertexBuffer, &memRequirements);

	VkMemoryAllocateInfo allocateInfo{};
	allocateInfo.sType												= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize										= memRequirements.size;
	allocateInfo.memoryTypeIndex									= __FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(Device, &allocateInfo, nullptr, &VertexBufferMemory) != VK_SUCCESS)
		throw std::runtime_error("Failed to create vertex buffer memory");

	vkBindBufferMemory(Device, VertexBuffer, VertexBufferMemory, 0);
}

//////////////////////////////////////////////////////////////////////////
// 比較 Swap Chain 的 Function
//////////////////////////////////////////////////////////////////////////
SwapChainSupportDetails VulkanEngineApplication::__QuerySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, Surface, &details.Capbilities);

	// Get Format
	uint32_t size = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, Surface, &size, nullptr);
	if (size > 0)
	{
		details.Formats.resize(size);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, Surface, &size, details.Formats.data());
	}

	// Get Present Mode
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, Surface, &size, nullptr);
	if (size > 0)
	{
		details.PresentModes.resize(size);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, Surface, &size, details.PresentModes.data());
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
	glfwGetFramebufferSize(Window, &width, &height);
	VkExtent2D actualExtent =
	{
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height)
	};
	actualExtent.width = clamp(static_cast<uint32_t>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	actualExtent.height = clamp(static_cast<uint32_t>(height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
	return actualExtent;
}
void VulkanEngineApplication::__CleanupSwapChain()
{
	#pragma region Frame Buffer
	for(auto &frameBuffer : SwapChainFrameBuffers)
		vkDestroyFramebuffer(Device, frameBuffer, nullptr);
	#pragma endregion
	#pragma region Image Views
	for(auto& imageView : SwapChainImageViews)
		vkDestroyImageView(Device, imageView, nullptr);
	#pragma endregion
	#pragma region Swap Cahin
	vkDestroySwapchainKHR(Device, SwapChain, nullptr);
	#pragma endregion
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
		for(const auto& currentProperties : layerProperties)
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