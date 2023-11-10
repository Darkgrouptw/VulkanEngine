#pragma once
#include "Core/Components/Scene/Data/ObjectBase.h"
#include "Core/Components/Scene/Data/Transform.h"

class Camera : public ObjectBase, public Transform
{
public:
    Camera(string);
    ~Camera();

    inline glm::mat4 GetProjectMatrix()                             { return mProjectMatrix; };
    inline glm::mat4 GetViewMatrix()                                {}
    inline void SetProjectMatrix(const glm::mat4 pMatrix) { mProjectMatrix = pMatrix; };

protected:
    glm::mat4 mProjectMatrix;
    glm::
};