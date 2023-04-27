#pragma once
#include "../Common/Common.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"

class IMGUIWindowManager
{
public:
    IMGUIWindowManager(GLFWwindow*);
    ~IMGUIWindowManager();

    void Render();
private:
    static ImGui_ImplVulkanH_Window WD;
};