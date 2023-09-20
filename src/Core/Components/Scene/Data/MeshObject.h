#pragma once
#include "Core/Components/Scene/Data/ObjectBase.h"

#include <tuple>
#include <vector>

class MeshObject : public ObjectBase
{
public:
    MeshObject();
    ~MeshObject();

    void InsertPositionAndNormal(glm::vec3, glm::vec3);
    void InsertPosition(glm::vec3);
    void InsertFaceIndex(uint32_t);

    uint32_t GetFaceIndicesSize();

protected:
    vector<glm::vec3> mPositions;
    vector<glm::vec3> mNormals;
    vector<uint32_t> mFaceIndices;
};