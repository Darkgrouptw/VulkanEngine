#pragma once
#include "Core/Common/Common.h"
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

    void InsertPositionAndNormal(glm::vec3, glm::vec3);
    void InsertPosition(glm::vec3);
    void InsertFaceIndex(int);
    void SetMaterialIndex(int);

    uint32_t GetFaceIndicesSize();

protected:
    vector<glm::vec3> mPositions;
    vector<glm::vec3> mNormals;
    vector<int> mFaceIndices;
    int mMaterialIndex;
};