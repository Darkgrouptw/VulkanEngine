#pragma once
#include "Core/Common/Common.h"

#include <vector>

using namespace std;

struct VertexBufferInfo
{
    glm::vec3 Position;                                                                                     // 頂點
    glm::vec3 Normal;                                                                                       // Normal
    glm::vec4 VertexColor;                                                                                  // 頂點顏色
    glm::vec2 Texcoord;                                                                                     // 貼圖座標

    // 設定 Vulkan 的 Vertex Input
    static vector<VkVertexInputAttributeDescription> GetAttributeDescription()
    {
        vector<VkVertexInputAttributeDescription> desc{};
        desc.resize(4);

        // Position
        desc[0].binding                                             = 0;
        desc[0].location                                            = 0;
        desc[0].format                                              = VK_FORMAT_R32G32B32_SFLOAT;
        desc[0].offset                                              = offsetof(VertexBufferInfo, Position); // 在 VertexBufferInfo 中， Position 是位於這個 class 的 offset 幾 byte

        // Normal
        desc[1].binding                                             = 0;
        desc[1].location                                            = 1;
        desc[1].format                                              = VK_FORMAT_R32G32B32_SFLOAT;
        desc[1].offset                                              = offsetof(VertexBufferInfo, Normal);

        // Vertex Color
        desc[2].binding                                             = 0;
        desc[2].location                                            = 2;
        desc[2].format                                              = VK_FORMAT_R32G32B32A32_SFLOAT;
        desc[2].offset                                              = offsetof(VertexBufferInfo, VertexColor);

        // Texture Coordinate
        desc[3].binding                                             = 0;
        desc[3].location                                            = 3;
        desc[3].format                                              = VK_FORMAT_R32G32_SFLOAT;
        desc[3].offset                                              = offsetof(VertexBufferInfo, Texcoord);

        return desc;
    }

    // 設定 Vulkan 的 Binding Description
    static VkVertexInputBindingDescription GetBindingDescription()
    {
        VkVertexInputBindingDescription desc{};
        desc.binding                                                = 0;
        desc.stride                                                 = sizeof(VertexBufferInfo);

        // VK_VERTEX_INPUT_RATE_VERTEX                              => 代表每一個點就是一包資料
        // VK_VERTEX_INPUT_RATE_INSTANCE                            => 代表每一 Instance 要一包資料
        desc.inputRate                                              = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;  
        return desc;
    }
};