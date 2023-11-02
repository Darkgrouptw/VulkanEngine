#pragma once
#include "Core/Common/Common.h"
#include "Core/Components/Scene/Data/ObjectBase.h"
#include "Core/Components/Scene/Data/Transform.h"

#include <unordered_map>

using namespace std;

class MaterialBase : public ObjectBase
{
public:
    MaterialBase(string);
    ~MaterialBase();

    glm::vec3 GetAmbientColor();
    glm::vec3 GetDiffuseColor();
    glm::vec3 GetSpecularColor();

    void SetAmbientColor(glm::vec3);
    void SetDiffuseColor(glm::vec3);
    void SetSpecularColor(glm::vec3);

    // ShaderType
    inline ShaderType GetShaderType() { return mShaderType; };
    void SetShaderType(ShaderType pType) { mShaderType = pType; };

protected:
    glm::vec3 mAmbientColor;
    glm::vec3 mDiffuseColor;
    glm::vec3 mSpecularColor;

    ShaderType mShaderType                                          = ShaderType::Unlit;
    //bool mIsTwoSide                                                 = false;

    //float mShininess                                                = 0.f;
    //float mReflectivitiy                                            = 0.f;
};
