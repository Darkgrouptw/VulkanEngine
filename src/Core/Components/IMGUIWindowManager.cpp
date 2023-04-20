#include "IMGUIWindowManager.h"

#pragma region Public
IMGUIWindowManager::IMGUIWindowManager()
{
    IMGUI_CHECKVERSION();

    ImGui::Begin("Hello World");
    ImGui::Text("AAA");
    ImGui::End();
}
IMGUIWindowManager::~IMGUIWindowManager()
{
}

void IMGUIWindowManager::Render()
{
    ImGui::Render();
}
#pragma endregion
#pragma region Private
#pragma endregion
