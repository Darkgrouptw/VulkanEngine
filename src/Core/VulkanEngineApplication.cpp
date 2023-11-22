#include "Core/VulkanEngineApplication.h"

VulkanEngineApplication* VulkanEngineApplication::Instance = nullptr;
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
	InitScene();
	MainLoop();
	Destroy();
}

// Windows Resize
// Static callback
void VulkanEngineApplication::ResizeCallback(GLFWwindow* pWindow, int pWidth, int pHeight)
{
	auto* app = reinterpret_cast<VulkanEngineApplication*>(glfwGetWindowUserPointer(pWindow));
	app->mFrameBufferResized = true;
}

// Vulkan Create Buffer
void VulkanEngineApplication::CreateBuffer(VkDeviceSize pSize, VkBufferUsageFlags pUsage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType												= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size													= pSize;
	bufferInfo.usage												= pUsage;
	bufferInfo.sharingMode											= VK_SHARING_MODE_EXCLUSIVE;			// 只有在 Graphics Queue 會用到，暫時先給 Exclusive

	if (vkCreateBuffer(mDevice, &bufferInfo, nullptr, &buffer)		!= VK_SUCCESS)
		throw std::runtime_error("Failed to create vertex buffer");

	// 設定 Buffer 內的大小
	VkMemoryRequirements memRequirements{};
	vkGetBufferMemoryRequirements(mDevice, buffer, &memRequirements);

	VkMemoryAllocateInfo allocateInfo{};
	allocateInfo.sType												= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize										= memRequirements.size;
	allocateInfo.memoryTypeIndex									= __FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(mDevice, &allocateInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		throw std::runtime_error("Failed to create vertex buffer memory");

	vkBindBufferMemory(mDevice, buffer, bufferMemory, 0);
}
void VulkanEngineApplication::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer buffer = __BeginSingleTimeCommand();
	{
		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		vkCmdCopyBuffer(buffer, srcBuffer, dstBuffer, 1, &copyRegion);
	}
	__EndSingleTimeCommand(buffer);
}

// Get Vulkan Item
void VulkanEngineApplication::GetViewportAndScissor(VkViewport& viewport, VkRect2D& scissor)
{
	// Viewport
	viewport.x 														= 0;
	viewport.y 														= 0;

	viewport.width 													= mSwapChainExtent.width;
	viewport.height													= mSwapChainExtent.height;
	viewport.minDepth												= 0;
	viewport.maxDepth												= 1;									// 設定 Depth 0 ~ 1

	// Scissor
	// https://vulkan-tutorial.com/images/viewports_scissors.png
	scissor.offset													= {0, 0};
	scissor.extent													= mSwapChainExtent;
}
#pragma endregion
#pragma region Private
void VulkanEngineApplication::InitWindow()
{
	// 設定 Instance
	VulkanEngineApplication::Instance = this;

	// 初始化
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);															// 預設初始化設定預設是 OpenGL，所以只用 NO_API
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);																// 設定不做 Resize

	Window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Engine", nullptr, nullptr);
	glfwSetWindowUserPointer(Window, this);
	glfwSetFramebufferSizeCallback(Window, VulkanEngineApplication::ResizeCallback);
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
	__CreateFrameBuffers();
	__CreateCommandPool();
	__CreateTextureImage();
	__CreateIMGUIDescriptor();
	__CreateCommandBuffer();
	__CreateSyncObjects();

	// 初始化 IMGUI
	ImGui_ImplVulkan_InitInfo initInfo{};
	initInfo.Instance 												= mInstance;
	initInfo.PhysicalDevice											= mPhysiclaDevice;
	initInfo.Device													= mDevice;
	initInfo.QueueFamily											= mIndices.GraphicsFamily.value();
	initInfo.Queue													= mGraphicsQueue;
	initInfo.PipelineCache											= VK_NULL_HANDLE;
	initInfo.DescriptorPool											= mImGuiDescriptorPool;
	initInfo.Subpass												= 0;

	SwapChainSupportDetails details 								= __QuerySwapChainSupport(mPhysiclaDevice);
	initInfo.MinImageCount											= details.Capbilities.minImageCount;
	initInfo.ImageCount												= SwapChainImages.size();
	initInfo.MSAASamples											= VK_SAMPLE_COUNT_1_BIT;
	initInfo.Allocator												= nullptr;
	initInfo.CheckVkResultFn										= nullptr;
	ImGuiWindowM													= new ImGuiWindowManager(Window, &initInfo, mRenderPass);
	ImGuiWindowM->FetchDeviceName(mPhysiclaDevice);
	ImGuiWindowM->UploadFont(mCommandPool, mGraphicsQueue, mDevice);
}
void VulkanEngineApplication::InitScene()
{
	SceneM = new SceneManager();
	SceneM->LoadScene("Scenes/Teapot/teapot.gltf");
	//SceneM->LoadScene("Scenes/CornellBox/CornellBox-Original.gltf");
	__ResetCameraAspect();
}
void VulkanEngineApplication::MainLoop()
{
	while (!glfwWindowShouldClose(Window))																	// 接到是否關閉此視窗的 Flag
	{
		glfwPollEvents();																					// 抓出 GFLW 的事件 Queue
		DrawFrame();
	}
	vkDeviceWaitIdle(mDevice);
}
void VulkanEngineApplication::Destroy()
{
	SceneM->UnloadScene();
	delete SceneM;

	// Delete ImGui
	delete ImGuiWindowM;

	#pragma region SwapChain
	__CleanupSwapChain();
	#pragma endregion
	#pragma region SyncObjects
	for (int i = 0; i < MAX_FRAME_IN_FLIGHTS; i++)
	{
		vkDestroySemaphore(mDevice, ImageAvailbleSemaphore[i], nullptr);
		vkDestroySemaphore(mDevice, RenderFinishedSemaphore[i], nullptr);
		vkDestroyFence(mDevice, InFlightFences[i], nullptr);
	}
	#pragma endregion
	#pragma region Command Pool
	vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
	#pragma endregion
	#pragma region IMGUI Descriptor Set Layout
	vkDestroyDescriptorPool(mDevice, mImGuiDescriptorPool, nullptr);
	#pragma endregion
	#pragma region Render Pass
	vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
	#pragma endregion
	#pragma region Device
	vkDestroyDevice(mDevice, nullptr);
	#pragma endregion
	#pragma region Surface
	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
	#pragma endregion
	#pragma region Debug Messager
	// 清掉 Vulkan 相關東西
#if defined(VKENGINE_DEBUG_DETAILS)
	if (EnabledValidationLayer)
		DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
#endif
	#pragma endregion
	#pragma region Instance
	vkDestroyInstance(mInstance, nullptr);
	#pragma endregion

	// 關閉 GLFW
	glfwDestroyWindow(Window);
	glfwTerminate();

	VulkanEngineApplication::Instance = nullptr;
}

void VulkanEngineApplication::DrawFrame()
{
	// 這你需要等待幾個步驟
	// 1. 等待前一幀的資料繪製完成
	// 2. 取得 Swap Chain 的圖片，更新 Uniform Buffer 和重新 Reset Fences
	// 3. 設定 Commands 到 Command Buffer 中來繪製整個場景到 Image 中
	// 4. Submit Command Buffer
	// 5. 顯示 Swap Chain Image
	#pragma region 1.
	vkWaitForFences(mDevice, 1, &InFlightFences[mCurrentFrameIndex], VK_TRUE, UINT64_MAX);
	#pragma endregion
	#pragma region 2.
	// 這裡要判斷是否已經過期
	// 有兩種狀況
	// 1. swap chain 的資料過期了 （通常發生在視窗大小改變，要重新建立新的 swap chain） 
	// 2. 可以繼續表現到 surface 上
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(mDevice, SwapChain, UINT64_MAX, ImageAvailbleSemaphore[mCurrentFrameIndex], VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		ReCreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) // VK_SUBOPTIMAL_KHR 代表，雖然可以正常顯示到 surface 上，但是 surface 其實不需要了
		throw runtime_error("Failed to acquire swap chain image");

	// 更新場景的所有資料
	SceneM->UpdateScene();

	vkResetFences(mDevice, 1, &InFlightFences[mCurrentFrameIndex]);											// Reset Fences
	#pragma endregion
	#pragma region 3.
	// Reset & Write
	vkResetCommandBuffer(mCommandBuffers[mCurrentFrameIndex], 0);											// 後面的參數 Flag，目前還不用，暫時先留 0
	__SetupCommandBuffer(mCommandBuffers[mCurrentFrameIndex], imageIndex);
	#pragma endregion
	#pragma region 4.
	VkSubmitInfo submitInfo{};
	submitInfo.sType												= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	
	// 等待 Semaphore 完成，在做 pass
	VkSemaphore waitSemaphores[] 									= { ImageAvailbleSemaphore[mCurrentFrameIndex] };
	VkPipelineStageFlags waitStages[]								= { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount									= static_cast<uint32_t>(sizeof(waitSemaphores) / sizeof(VkSemaphore));
	submitInfo.pWaitSemaphores										= waitSemaphores;
	submitInfo.pWaitDstStageMask									= waitStages;

	submitInfo.commandBufferCount									= 1;
	submitInfo.pCommandBuffers										= &mCommandBuffers[mCurrentFrameIndex];

	// 完成此 Submit 要觸發 singalSempahore
	VkSemaphore signalSemphores[]									= { RenderFinishedSemaphore[mCurrentFrameIndex] };
	submitInfo.signalSemaphoreCount									= static_cast<uint32_t>(sizeof(signalSemphores) / sizeof(VkSemaphore));
	submitInfo.pSignalSemaphores									= signalSemphores;

	if (vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, InFlightFences[mCurrentFrameIndex]) != VK_SUCCESS)
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
	
	result															= vkQueuePresentKHR(mGraphicsQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mFrameBufferResized)
	{
		mFrameBufferResized = false;
		ReCreateSwapChain();
	}
	else if (result != VK_SUCCESS)
		throw runtime_error("Failed to present swap chan image");

	// 切換下一張
	mCurrentFrameIndex = (mCurrentFrameIndex + 1) % MAX_FRAME_IN_FLIGHTS;
	#pragma endregion
}
void VulkanEngineApplication::ReCreateSwapChain()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(Window, &width, &height);

	// 畫面最小化的時不畫
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(Window, &width, &height);
		glfwWaitEvents();
	}

	// 等待動作都 Idle 完之後，再繼續做清除的動作
	vkDeviceWaitIdle(mDevice);

	__CleanupSwapChain();
	__CreateSwapChain();
	__CreateImageViews();
	__CreateFrameBuffers();

	__ResetCameraAspect();
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
	VkResult result													= vkCreateInstance(&createInfo, nullptr, &mInstance);
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
		if (CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessenger) != VK_SUCCESS)
    		throw std::runtime_error("Failed to set up debug messenger");
	}
#endif
}
void VulkanEngineApplication::__CreateSurface()
{
	VkResult result = glfwCreateWindowSurface(mInstance, Window, nullptr, &mSurface);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create window surface");
}
void VulkanEngineApplication::__PickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);											// 抓出所有支援的 Device
	if (deviceCount == 0)
		throw runtime_error("Failed to find GPUs with Vulkan Support");

	// 並非所有的顯卡都符合設定
	// 所以這邊要做更進一步的 Check (例如是否有 Geometry Shader 等)
	vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());
	for(const auto& device: devices)
		if (__IsDeviceSuitable(device))
		{
			mPhysiclaDevice = device;
			break;
		}
	
	if (mPhysiclaDevice == VK_NULL_HANDLE)
		throw runtime_error("No Suitable GPUs");
}
void VulkanEngineApplication::__CreateLogicalDevice()
{
	mIndices															= __FindQueueFamilies(mPhysiclaDevice);

	vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	set<uint32_t> uniqueQueueFamilies 								= {mIndices.GraphicsFamily.value(), mIndices.PresentFamily.value()};

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
	deviceFeatures.samplerAnisotropy								= VK_TRUE;								// 開啟 Anisotropy

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
	if (vkCreateDevice(mPhysiclaDevice, &createInfo, nullptr, &mDevice) != VK_SUCCESS)
		throw runtime_error("Failed to create logical device");
    vkGetDeviceQueue(mDevice, mIndices.GraphicsFamily.value(), 0, &mGraphicsQueue);
    vkGetDeviceQueue(mDevice, mIndices.PresentFamily.value(), 0, &mPresentQueue);
}
void VulkanEngineApplication::__CreateSwapChain()
{
	SwapChainSupportDetails details 								= __QuerySwapChainSupport(mPhysiclaDevice);
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
	createInfo.surface												= mSurface;

	// 傳入上方的設定
	createInfo.imageColorSpace										= surfaceFormat.colorSpace;
	createInfo.imageFormat											= surfaceFormat.format;
	createInfo.presentMode											= presentMode;
	createInfo.imageExtent											= extent;
	createInfo.minImageCount										= imageCount;

	// 其他細節設定
	createInfo.imageArrayLayers										= 1;									// 如果要使用 Stereo 就會需要兩個 (兩個輸出)，不然一般都是一個
	createInfo.imageUsage											= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;	// 輸出的圖，可以做什麼使用的設定
	uint32_t queueFamilyIndices[]									= { mIndices.GraphicsFamily.value(), mIndices.PresentFamily.value() };
	
	// 判斷 Graphics & Present 是否在同一個 Queue
	if (mIndices.GraphicsFamily.value() != mIndices.PresentFamily.value())
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
	if (vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &SwapChain) != VK_SUCCESS)
		throw new runtime_error("Failed to create SwapChain");
	
	vkGetSwapchainImagesKHR(mDevice, SwapChain, &imageCount, nullptr);
	SwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(mDevice, SwapChain, &imageCount, SwapChainImages.data());

	SwapChainImageFormat 											= surfaceFormat.format;
	mSwapChainExtent												= extent;
}
void VulkanEngineApplication::__CreateImageViews()
{
	SwapChainImageViews.resize(SwapChainImages.size());
	for (size_t i = 0; i < SwapChainImages.size(); i++)
		SwapChainImageViews[i] = __CreateImageView(SwapChainImages[i], SwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

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

	if (vkCreateRenderPass(mDevice, &renderPassInfo, nullptr, &mRenderPass) != VK_SUCCESS)
		throw runtime_error("Failed to create render pass");
}
void VulkanEngineApplication::__CreateFrameBuffers()
{
	SwapChainFrameBuffers.resize(SwapChainImageViews.size());
	for (size_t i = 0; i < SwapChainImageViews.size(); i++)
	{
		VkImageView attachments[] 									= { SwapChainImageViews[i] };
		VkFramebufferCreateInfo frameBufferInfo{};
		frameBufferInfo.sType										= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferInfo.renderPass									= mRenderPass;
		frameBufferInfo.attachmentCount								= static_cast<uint32_t>(sizeof(attachments) / sizeof(VkImageView));
		frameBufferInfo.pAttachments								= attachments;
		frameBufferInfo.width										= mSwapChainExtent.width;
		frameBufferInfo.height										= mSwapChainExtent.height;
		frameBufferInfo.layers										= 1;

		if (vkCreateFramebuffer(mDevice, &frameBufferInfo, nullptr, &SwapChainFrameBuffers[i]) != VK_SUCCESS)
			throw runtime_error("Failed to create framebuffer");
	}
}
void VulkanEngineApplication::__CreateDepthBuffers()
{
	VkFormat depthFormat = __GetDepthFormat();
	mDepthImageView = __CreateImageView(mDepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}
void VulkanEngineApplication::__CreateCommandPool()
{
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType													= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags													= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex										= mIndices.GraphicsFamily.value();

	if (vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mCommandPool) != VK_SUCCESS)
		throw runtime_error("Failed to create command pool");
}
void VulkanEngineApplication::__CreateTextureImage()
{
	/*auto lambdaCreateBufferFunction = [&](VkDeviceSize dataSize, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		__CreateBuffer(
			dataSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			buffer,
			bufferMemory
		);
	};
	auto lambdaFindMemoryTypeFunction = [&](uint32_t typeFiler, VkMemoryPropertyFlags properties) -> uint32_t 
	{
		return __FindMemoryType(typeFiler, properties);
	};
	auto lambdaBeginSingleTimeCommandFunction = [&]() -> VkCommandBuffer
	{
		return __BeginSingleTimeCommand();
	};
	auto lambdaEndSingleTimeCommandFunction = [&](VkCommandBuffer buffer)
	{
		__EndSingleTimeCommand(buffer);
	};
	TextM = new TextureManager(
		"Textures/texture.jpg",
		lambdaCreateBufferFunction, Device,
		lambdaFindMemoryTypeFunction,
		lambdaBeginSingleTimeCommandFunction,
		lambdaEndSingleTimeCommandFunction,
		VK_FORMAT_R8G8B8A8_SRGB);
	TextM->CreateImageView();
	TextM->CreateSampler(PhysiclaDevice);*/
}
void VulkanEngineApplication::__CreateIMGUIDescriptor()
{
    #pragma region Description Pool For ImGui
    VkDescriptorPoolSize imGuiPoolSizes[] = {
		{VK_DESCRIPTOR_TYPE_SAMPLER,                                1000},
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,                 1000},
		{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,                          1000},
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,                          1000},
		{VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,                   1000},
		{VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,                   1000},
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,                         1000},
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,                         1000},
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,                 1000},
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,                 1000},
		{VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,                       1000}};

    VkDescriptorPoolCreateInfo imGuiPoolInfo{};
    imGuiPoolInfo.sType                                           	= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    imGuiPoolInfo.flags                                           	= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    imGuiPoolInfo.maxSets                                         	= 1000 * IM_ARRAYSIZE(imGuiPoolSizes);
    imGuiPoolInfo.poolSizeCount                                   	= static_cast<uint32_t>(IM_ARRAYSIZE(imGuiPoolSizes));
    imGuiPoolInfo.pPoolSizes                                      	= imGuiPoolSizes;

    if (vkCreateDescriptorPool(mDevice, &imGuiPoolInfo, nullptr, &mImGuiDescriptorPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create DescriptorPool for ImGuiDescriptorPool");
    #pragma endregion
}
void VulkanEngineApplication::__CreateCommandBuffer()
{
	mCommandBuffers.resize(MAX_FRAME_IN_FLIGHTS);
	
	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType												= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool										= mCommandPool;
	allocateInfo.level												= VK_COMMAND_BUFFER_LEVEL_PRIMARY;		// 如果是 Primary，代表直接送 Command Buffer，無法被其他 Command Buffer 讀取
	allocateInfo.commandBufferCount									= static_cast<uint32_t>(mCommandBuffers.size());

	if (vkAllocateCommandBuffers(mDevice, &allocateInfo, mCommandBuffers.data()) != VK_SUCCESS)
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
		if (vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &ImageAvailbleSemaphore[i]) 				!= VK_SUCCESS ||
			vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &RenderFinishedSemaphore[i]) 				!= VK_SUCCESS ||
			vkCreateFence(mDevice, &fenceInfo, nullptr, &InFlightFences[i])									!= VK_SUCCESS)
			throw runtime_error("Failed to create sync objects");
	}
}

//////////////////////////////////////////////////////////////////////////
// Helper Render Function
//////////////////////////////////////////////////////////////////////////
void VulkanEngineApplication::__SetupCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType													= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		throw runtime_error("Failed to begin recording command buffer");

	// 設定 Render Pass
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType											= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass										= mRenderPass;
	renderPassInfo.framebuffer										= SwapChainFrameBuffers[imageIndex];
	renderPassInfo.renderArea.offset								= { 0, 0 };
	renderPassInfo.renderArea.extent								= mSwapChainExtent;

	// Clear
	VkClearValue clearColor 										= {{{ 0.2f, 0.84f, 0.8f, 1 }}};
	renderPassInfo.clearValueCount									= 1;
	renderPassInfo.pClearValues										= &clearColor;

	// 設定 RenderPass (且無其他 Pass => CONTENTS_INLINE)
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	{
		// 由於前面設定 VkPipelineDynamicStateCreateInfo
		// 設定了 VK_DYNAMIC_STATE_VIEWPORT & VK_DYNAMIC_STATE_SCISSOR
		// 所以這裡需要在指定一次
		VkViewport viewport{};
		VkRect2D scissor{};
		GetViewportAndScissor(viewport, scissor);

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		// Render
		SceneM->RenderScene(commandBuffer);
		ImGuiWindowM->Render(commandBuffer);
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

	// 抓取顯卡的 Feature
	// 並抓取是否可以成功開啟 samplerAnisotropy
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	if (!deviceFeatures.samplerAnisotropy)
		return false;

	// 測試顯卡的一些細節
#if defined(VKENGINE_DEBUG_DETAILS)
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

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
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface, &presentSupport);
		if (presentSupport)
			indices.PresentFamily = i;

		if (indices.IsCompleted())
			break;
	}
	return indices;																								// 無正常的可以處理 Graphics 的 Queue */
}
uint32_t VulkanEngineApplication::__FindMemoryType(uint32_t typeFiler, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(mPhysiclaDevice, &memProperties);

	for(uint32_t i = 0 ; i < memProperties.memoryTypeCount; i++)
		if (typeFiler & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	throw runtime_error("Failed to find suitable memory type");
}
VkImageView VulkanEngineApplication::__CreateImageView(VkImage pImage, VkFormat pFormat, VkImageAspectFlags pAspectFlags)
{
	// 相同於 TextureManager::CreateImageView
	VkImageViewCreateInfo createInfo{};
	createInfo.sType 												= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image												= pImage;
	createInfo.viewType												= VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format												= pFormat;

	// Image 的 Range 設定 0 ~ 1
	createInfo.components.r											= VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g											= VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b											= VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a											= VK_COMPONENT_SWIZZLE_IDENTITY;

	// 其他用途的設定 (Mipmap 等)
	createInfo.subresourceRange.aspectMask							= pAspectFlags;
	createInfo.subresourceRange.baseMipLevel						= 0;
	createInfo.subresourceRange.levelCount							= 1;
	createInfo.subresourceRange.baseArrayLayer						= 0;
	createInfo.subresourceRange.layerCount							= 1;

	// Create Image
	VkImageView imageView;
	if (vkCreateImageView(mDevice, &createInfo, nullptr, &imageView) != VK_SUCCESS)
		throw runtime_error("Failed to create ImageView");
	return imageView;
}

//////////////////////////////////////////////////////////////////////////
// Texture Format
//////////////////////////////////////////////////////////////////////////
VkFormat VulkanEngineApplication::__GetDepthFormat()
{
	// VK_FORMAT_D32_SFLOAT: 32-bit float for depth
	// VK_FORMAT_D32_SFLOAT_S8_UINT: 32 - bit signed float for depth and 8 bit stencil component
	// VK_FORMAT_D24_UNORM_S8_UINT : 24 - bit float for depth and 8 bit stencil component
	return __FindSupportedTextureFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}
VkFormat VulkanEngineApplication::__FindSupportedTextureFormat(const vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(mPhysiclaDevice, format, &props);

		// 有三種 VkFormatProperties
		// 1. linearTilingFeatures: Use cases that are supported with linear tiling
		// 2. optimalTilingFeatures : Use cases that are supported with optimal tiling
		// 3. bufferFeatures : Use cases that are supported for buffers
		if (tiling == VK_IMAGE_TILING_LINEAR						&& (props.linearTilingFeatures & features) == features)
			return format;
		else if (tiling == VK_IMAGE_TILING_OPTIMAL					&& (props.optimalTilingFeatures & features) == features)
			return format;
	}
	throw runtime_error("Failed to find supported texture format!");
}
bool VulkanEngineApplication::__HasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkCommandBuffer VulkanEngineApplication::__BeginSingleTimeCommand()
{
	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType												= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.level												= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandPool										= mCommandPool;
	allocateInfo.commandBufferCount									= 1;

	VkCommandBuffer commandBuffer;
	if (vkAllocateCommandBuffers(mDevice, &allocateInfo, &commandBuffer) != VK_SUCCESS)
		throw runtime_error("Failed to allocate buffer for allocation");

	// Command Buffer
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType													= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags													= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;	// 只 Copy 一次
	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	return commandBuffer;
}
void VulkanEngineApplication::__EndSingleTimeCommand(VkCommandBuffer pBuffer)
{
	vkEndCommandBuffer(pBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType												= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount									= 1;
	submitInfo.pCommandBuffers										= &pBuffer;
	
	vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(mGraphicsQueue);

	// Free Buffer
	vkFreeCommandBuffers(mDevice, mCommandPool, 1, &pBuffer);
}

//////////////////////////////////////////////////////////////////////////
// 比較 Swap Chain 的 Function
//////////////////////////////////////////////////////////////////////////
SwapChainSupportDetails VulkanEngineApplication::__QuerySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mSurface, &details.Capbilities);

	// Get Format
	uint32_t size = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &size, nullptr);
	if (size > 0)
	{
		details.Formats.resize(size);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &size, details.Formats.data());
	}

	// Get Present Mode
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &size, nullptr);
	if (size > 0)
	{
		details.PresentModes.resize(size);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &size, details.PresentModes.data());
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
		vkDestroyFramebuffer(mDevice, frameBuffer, nullptr);
	#pragma endregion
	#pragma region Image Views
	for(auto& imageView : SwapChainImageViews)
		vkDestroyImageView(mDevice, imageView, nullptr);
	#pragma endregion
	#pragma region Swap Cahin
	vkDestroySwapchainKHR(mDevice, SwapChain, nullptr);
	#pragma endregion
}
void VulkanEngineApplication::__ResetCameraAspect() 
{
	float aspect = (float)mSwapChainExtent.width / mSwapChainExtent.height;
	SceneM->SetCameraAspect(aspect);
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