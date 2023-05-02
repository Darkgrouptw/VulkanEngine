#include "IMGUIWindowManager.h"

#pragma region Public
IMGUIWindowManager::IMGUIWindowManager(GLFWwindow *window, ImGui_ImplVulkan_InitInfo* info, VkRenderPass& pass)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // 手把和鍵盤支援
    //ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_Init(info, pass);
}
IMGUIWindowManager::~IMGUIWindowManager()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void IMGUIWindowManager::Render()
{
    // ImGui_ImplVulkan_NewFrame();
    // ImGui_ImplGlfw_NewFrame();
    // ImGui::NewFrame();

    // ImGui::Begin("Hello World");
    // ImGui::Text("AAA");
    // ImGui::End();
    // ImGui::Render();
}
#pragma endregion
#pragma region Private
#pragma endregion
