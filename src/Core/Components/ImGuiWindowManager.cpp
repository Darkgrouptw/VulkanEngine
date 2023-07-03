#include "Core/Components/ImGuiWindowManager.h"

#pragma region Public
ImGuiWindowManager::ImGuiWindowManager(GLFWwindow *pWindow, ImGui_ImplVulkan_InitInfo* pInfo, VkRenderPass& pPass)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // 手把和鍵盤支援
    //ImGuiIO& io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForVulkan(pWindow, true);
    ImGui_ImplVulkan_Init(pInfo, pPass);
}
ImGuiWindowManager::~ImGuiWindowManager()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiWindowManager::FetchDeviceName(VkPhysicalDevice& pDevice)
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(pDevice, &properties);

    mDeviceName = string(properties.deviceName);
}
void ImGuiWindowManager::UploadFont(VkCommandPool& pool, VkQueue& queue, VkDevice& device)
{
    #pragma region Command Buffer
    VkCommandBuffer buffer;
    VkCommandBufferAllocateInfo info = {};
    info.sType                                                      = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.commandPool                                                = pool;
    info.level                                                      = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandBufferCount                                         = 1;

    if (vkAllocateCommandBuffers(device, &info, &buffer)            != VK_SUCCESS)
        throw runtime_error("Failed to create command buffer for IMGUI");
    #pragma endregion
    #pragma region Upload Font
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType                                                 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags                                                 |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(buffer, &beginInfo)                    != VK_SUCCESS)
        throw runtime_error("Failed to create buffer to upload IMGUI font");
    
    // Create font
    ImGui_ImplVulkan_CreateFontsTexture(buffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType                                                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount                                   = 1;
    submitInfo.pCommandBuffers                                      = &buffer;
    if (vkEndCommandBuffer(buffer)                                  != VK_SUCCESS)
        throw runtime_error("Failed to end buffer to upload IMGUI font");
        
    if (vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE)        != VK_SUCCESS)
        throw runtime_error("Failed to submit buffer to upload IMGUI font");

    if (vkDeviceWaitIdle(device)                                    != VK_SUCCESS)
        throw runtime_error("Failed to wait idle from uploading IMGUI font");
    ImGui_ImplVulkan_DestroyFontUploadObjects();
    #pragma endregion
}
void ImGuiWindowManager::Render(VkCommandBuffer commandBuffer)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // 畫 Debug 視窗
    DebugToolBox();

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}
#pragma endregion
#pragma region Private
void ImGuiWindowManager::DebugToolBox()
{
    ImGuiIO& io = ImGui::GetIO();
    
    ImGui::Begin("Debug toolbox");
    ImGui::BeginGroup();
    {
        ImGui::Text("Device: %s", mDeviceName.c_str());
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    }
    ImGui::EndGroup();
    ImGui::End();
}
#pragma endregion
