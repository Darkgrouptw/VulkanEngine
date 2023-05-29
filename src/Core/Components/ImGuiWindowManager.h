#pragma once
#include "../Common/Common.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"

#include <stdexcept>

using namespace std;

class ImGuiWindowManager
{
public:
    ImGuiWindowManager(GLFWwindow*, ImGui_ImplVulkan_InitInfo*, VkRenderPass&);
    ~ImGuiWindowManager();

    void UploadFont(VkCommandPool&, VkQueue&, VkDevice&);
    void Render();
private:
    ImGuiIO io;
};