#pragma once
#include "../Common/Common.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"

#include <stdexcept>

using namespace std;

class IMGUIWindowManager
{
public:
    IMGUIWindowManager(GLFWwindow*, ImGui_ImplVulkan_InitInfo*, VkRenderPass&);
    ~IMGUIWindowManager();

    void UploadFont(VkQueue&, VkDevice&);
    void Render();
private:

    static ImGui_ImplVulkanH_Window WD;
};