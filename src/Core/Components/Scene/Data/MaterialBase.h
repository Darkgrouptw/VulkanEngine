#pragma once
#include "Core/Common/Common.h"
#include "Core/Components/Scene/Data/ObjectBase.h"
#include "Core/Components/Scene/Data/Transform.h"
#include "Core/Components/Scene/Data/Shader/ShaderType.h"

#include <unordered_map>

using namespace std;

class MaterialBase : public ObjectBase
{
public:
    MaterialBase(string);
    ~MaterialBase();

    inline glm::vec4 GetAmbientColor()                              { return mAmbientColor; };
    inline glm::vec4 GetDiffuseColor()                              { return mDiffuseColor; };
    inline glm::vec4 GetSpecularColor()                             { return mSpecularColor; };

    inline void SetAmbientColor(const glm::vec4 pColor)             { mAmbientColor = pColor; };
    inline void SetDiffuseColor(const glm::vec4 pColor)             { mDiffuseColor = pColor; };
    inline void SetSpecularColor(const glm::vec4 pColor)            { mSpecularColor = pColor; };

    // ShaderType
    inline ShaderType GetShaderType()                               { return mShaderType; };
    inline void SetShaderType(ShaderType pType)                     { mShaderType = pType; };

protected:
    glm::vec4 mAmbientColor;
    glm::vec4 mDiffuseColor;
    glm::vec4 mSpecularColor;

    ShaderType mShaderType                                          = ShaderType::Unlit;
    //bool mIsTwoSide                                                 = false;

    //float mShininess                                                = 0.f;
    //float mReflectivitiy                                            = 0.f;
};
