#include "../Common/Common.h"

namespace VulkanEngine
{
	/// <summary>
	/// 管理 GLFW 的 Window
	/// </summary>
	class WindowManager
	{
	public:
		WindowManager();
		~WindowManager();

		void CreateWindowSurface(VkInstance&, VkSurfaceKHR*);												// 建立和視窗溝通的 Surface (GLFW & Vulkan)
		
		VkExtent2D GetExtentSize();																			// 抓取視窗大小，避免以後有可以更變視窗的問題
		bool ShouldClose();																					// 從 GLFW 接到是否關閉此視窗的 Flag

	private:
		// 視窗設定
		GLFWwindow* Window											= NULL;									// GLFW Window
		const int WIDTH												= 1600;									// 長
		const int HEIGHT											= 900;									// 寬
	};
}