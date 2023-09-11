#pragma once
#include "Core/Components/Scene/Data/ObjectBase.h"

#include <vector>

class MeshObject : public ObjectBase
{
public:
    MeshObject();
    ~MeshObject();

protected:
    vector<glm::vec3> mPositions;
    vector<glm::vec2> mNormals;
    //vector<>
};