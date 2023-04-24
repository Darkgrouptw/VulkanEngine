#include "IMGUIWindowManager.h"

#pragma region Public
IMGUIWindowManager::IMGUIWindowManager()
{
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    ImGui::StyleColorsDark();
}
IMGUIWindowManager::~IMGUIWindowManager()
{
}

void IMGUIWindowManager::Render()
{
    ImGui::NewFrame();
    ImGui::Begin("Hello World");
    ImGui::Text("AAA");
    ImGui::End();
    ImGui::Render();
}
#pragma endregion
#pragma region Private
#pragma endregion
