#pragma once
#include "Core/Components/Scene/Data/ObjectBase.h"

#include <vector>

class MeshObject : public ObjectBase
{
public:
    MeshObject();
    ~MeshObject();

    void InsertPositionAndNormal(glm::vec3, glm::vec2);
    void InsertPosition(glm::vec3);

protected:
    vector<glm::vec3> mPositions;
    vector<glm::vec2> mNormals;
};