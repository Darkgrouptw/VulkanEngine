#pragma once
#include "Core/Components/Scene/Data/ObjectBase.h"

#include <string>
#include <tuple>
#include <vector>

using namespace std;

class MeshObject : public ObjectBase
{
public:
    MeshObject(string, int);
    ~MeshObject();

    void InsertPositionAndNormal(glm::vec3, glm::vec3);
    void InsertPosition(glm::vec3);
    void InsertFaceIndex(int);

    uint32_t GetFaceIndicesSize();

protected:
    string mName;
    vector<glm::vec3> mPositions;
    vector<glm::vec3> mNormals;
    vector<int> mFaceIndices;
    int mMaterialIndex;

};