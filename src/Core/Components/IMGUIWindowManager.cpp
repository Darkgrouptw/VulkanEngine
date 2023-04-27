#include "IMGUIWindowManager.h"

#pragma region Public
IMGUIWindowManager::IMGUIWindowManager(GLFWwindow *window)
{
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(window, true);
}
IMGUIWindowManager::~IMGUIWindowManager()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void IMGUIWindowManager::Render()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Hello World");
    ImGui::Text("AAA");
    ImGui::End();
    ImGui::Render();
}
#pragma endregion
#pragma region Private
#pragma endregion
