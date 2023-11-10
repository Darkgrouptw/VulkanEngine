#pragma once
#include "Core/Common/Common.h"
#include "Core/Components/Scene/Data/ObjectBase.h"
#include "Core/Components/Scene/Data/Transform.h"

class Camera : public ObjectBase, public Transform
{
public:
    Camera(string);
    ~Camera();

    inline glm::mat4 GetProjectMatrix()                             { return mProjectMatrix; };
    inline glm::mat4 GetViewMatrix()                                { return glm::lookAt( Position, GetLookAtPos(), GetLookUp()); };
    inline void SetProjectMatrix(const glm::mat4 pMatrix)           { mProjectMatrix = pMatrix; };
    //inline void SetLookAtPos(const glm::vec3 pPos, const glm::vec3 pUp) { mLookAtPos = pPos; mLookAtUp = pUp; };

protected:
    glm::mat4 mProjectMatrix;
    inline glm::vec3 GetLookAtPos()                                 { return glm::toMat4(Rotation) * Common::VectorForward; };
    inline glm::vec3 GetLookUp()                                    { return glm::}
};