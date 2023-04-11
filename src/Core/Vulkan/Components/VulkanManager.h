#include "../Common/Common.h"

namespace VulkanEngine
{
	/// <summary>
	/// 管理所有的 Vulkan 的 Manager
	/// </summary>
	class VulkanManager
	{
	public:
		VulkanManager();
		~VulkanManager();

		VkInstance& GetVKInstance();

	private:
		void CreateVKInstance();																			// 設定 Vulkan 的 Instance
		void SetupDebugMessenger();																			// 設定 Debug 訊息

		// Vulkan 資料
		VkInstance Instance;
		VkDebugUtilsMessengerEXT DebugMessenger;

		//////////////////////////////////////////////////////////////////////////
		// Helper Function
		//////////////////////////////////////////////////////////////////////////
		void __PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT&);						// 設定 Type


		//////////////////////////////////////////////////////////////////////////
		// Debug Vulkan 的功能
		//////////////////////////////////////////////////////////////////////////
#if defined(VKENGINE_DEBUG_DETAILS)
		bool EnabledValidationLayer 								= false;								// 是否底層有支援這個功能
		vector<const char*> ValidationLayersNames 					= 										// 判斷是否有底下的標籤，就代表是否可以開啟這個功能
		{
			"VK_LAYER_KHRONOS_validation"
		};
		bool __CheckValidationLayerSupport();																// 檢查 Vulkan Validtion Layer 是否支援
#endif
	};
}