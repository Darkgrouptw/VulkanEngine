#pragma once
#include "Core/Common/Common.h"
#include "Core/Components/Buffer/VertexBufferInfo.h"
#include "Core/Components/Scene/Data/ObjectBase.h"
#include "Core/Components/Scene/Data/Transform.h"
#include "Core/Components/Scene/Data/VulkanInterface.h"

#include <string>
#include <tuple>
#include <vector>

using namespace std;

class MeshObject : public ObjectBase, public Transform, VulkanInterface
{
public:
    MeshObject(string, int);
    ~MeshObject();

    void InsertVertexData(glm::vec3, glm::vec3, glm::vec4, glm::vec2);
    void InsertFaceIndex(uint32_t);
    inline int GetMaterialIndex()                                   { return mMaterialIndex; };
    inline void SetMaterialIndex(int pIndex)                        { mMaterialIndex = pIndex; };

    uint32_t GetFaceIndicesSize();
    glm::mat4 GetModelMatrix();

    // Vulkan Stuff
    void CreateVulkanStuff() override;
    void DestroyVulkanStuff() override;
    void Render(const VkCommandBuffer, const VkPipelineLayout, const VkDescriptorSet&);

protected:
    vector<VertexBufferInfo> mVertices;
    vector<uint32_t> mFaceIndices;
    int mMaterialIndex;

    // Vulkan Buffer
    VkBuffer mVertexBuffer = nullptr, mIndexBuffer = nullptr;
    VkDeviceMemory mVertexBufferMemory = nullptr, mIndexBufferMemory = nullptr;

	// Vulkan Create Command
	void CreateVertexBuffer();
	void CreateIndexBuffer();

    // Vulkan Destroy Command
	void DestroyVertexBuffer();
	void DestroyIndexBuffer();
};