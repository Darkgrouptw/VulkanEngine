#include "../Common/Common.h"

#include <glm/glm.hpp>
#include <vector>

using namespace std;

struct VertexBuffer
{
    glm::vec2 Position;
    glm::vec3 Color;


    // 設定 Vulkan 的 Vertex Input
    static vector<VkVertexInputAttributeDescription> GetAttributeDescription()
    {
        vector<VkVertexInputAttributeDescription> desc{};
        desc.resize(2);

        // Position
        desc[0].binding                                             = 0;
        desc[0].location                                            = 0;
        desc[0].format                                              = VK_FORMAT_R32G32_SFLOAT;
        desc[0].offset                                              = offsetof(VertexBuffer, Position);     // 在 VertexBuffer 中， Position 是位於這個 class 的 offset 幾 byte

        // Color
        desc[1].binding                                             = 0;
        desc[1].location                                            = 1;
        desc[1].format                                              = VK_FORMAT_R32G32B32_SFLOAT;
        desc[1].offset                                              = offsetof(VertexBuffer, Color);
        return desc;
    }

    // 設定 Vulkan 的 Binding Description
    static VkVertexInputBindingDescription GetBindingDescription()
    {
        VkVertexInputBindingDescription desc{};
        desc.binding                                                = 0;
        desc.stride                                                 = sizeof(VertexBuffer);

        // VK_VERTEX_INPUT_RATE_VERTEX                              => 代表每一個點就是一包資料
        // VK_VERTEX_INPUT_RATE_INSTANCE                            => 代表每一 Instance 要一包資料
        desc.inputRate                                              = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;  
        return desc;
    }
};

const std::vector<VertexBuffer> vertices = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};