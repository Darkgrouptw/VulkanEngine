#include "WindowManager.h"

namespace VulkanEngine
{
#pragma region Public
	WindowManager::WindowManager()
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
	WindowManager::~WindowManager()
	{

		// 關閉 GLFW
		glfwDestroyWindow(Window);
		glfwTerminate();
	}

	bool WindowManager::ShouldClose()
	{
		return glfwWindowShouldClose(Window);
	}

	void WindowManager::CreateWindowSurface(VkInstance& instance, VkSurfaceKHR* surface)
	{
		VkResult result = glfwCreateWindowSurface(instance, Window, nullptr, surface);
		if (result != VK_SUCCESS)
			throw runtime_error("Failed to create window surface");
	}
	VkExtent2D WindowManager::GetExtentSize()
	{
		int width, height;
		glfwGetFramebufferSize(Window, &width, &height);
		VkExtent2D actualExtent =
		{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};
		return actualExtent;
	}

#pragma endregion
#pragma region Private
#pragma endregion
}