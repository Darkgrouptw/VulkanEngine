#pragma once
#include "Core/Common/Common.h"
#include "Core/Components/Scene/Data/ObjectBase.h"
#include "Core/Components/Scene/Data/Transform.h"

class Camera : public ObjectBase, public Transform
{
public:
    Camera(string);
    ~Camera();

    glm::mat4 GetProjectMatrix();
    inline glm::mat4 GetViewMatrix()                                { return glm::lookAt( Position, GetLookAtPos(), GetLookUp()); };
    inline void SetCameraAspect(const float pAspect)                { mAspect = pAspect; };

protected:
    float mAspect                                                   = 16.0f / 9;
    inline glm::vec3 GetLookAtPos()                                 { return glm::mat4_cast(Rotation) * glm::vec4(Common::VectorForward(), 1.f); };
    inline glm::vec3 GetLookUp()                                    { return glm::mat4_cast(Rotation) * glm::vec4(Common::VectorUp(), 1.f); };

    // Const Parameters
    const float mFov                                                = 60.f;
    const float mNear                                               = 0.1f;
    const float mFar                                                = 1000.f;
};