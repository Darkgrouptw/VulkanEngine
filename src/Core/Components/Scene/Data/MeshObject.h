#pragma once
#include "Core/Common/Common.h"
#include "Core/Components/VertexBufferInfo.h"
#include "Core/Components/Scene/Data/ObjectBase.h"
#include "Core/Components/Scene/Data/Transform.h"

#include <string>
#include <tuple>
#include <vector>

using namespace std;

class MeshObject : public ObjectBase, Transform
{
public:
    MeshObject(string, int);
    ~MeshObject();

    void InsertVertexData(glm::vec3, glm::vec3, glm::vec2, glm::vec3);
    void InsertVertexData(glm::vec3, glm::vec3, glm::vec2);
    void InsertVertexData(glm::vec3, glm::vec3);
    void InsertVertexData(glm::vec3);
    void InsertFaceIndex(int);
    void SetMaterialIndex(int);

    uint32_t GetFaceIndicesSize();

    // Vulkan Command
    void CreateVertexBuffer();

protected:
    vector<VertexBufferInfo> mVertices;
    vector<int> mFaceIndices;
    int mMaterialIndex;


    VkBuffer mVertexBuffer, mIndexBuffer;
    VkDeviceMemory mVertexBufferMemory, mIndexBufferMemory;
};