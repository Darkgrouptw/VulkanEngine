#pragma once
#include "Core/Common/Common.h"
#include "Core/Components/Scene/Data/GLTFShaderTypeUtil.h"
#include "Core/Components/Scene/Data/ObjectBase.h"

//#include <string>

using namespace std;

class ShaderBase : public ObjectBase
{
public:
    ShaderBase(string, ShaderType);
    ~ShaderBase();

    // Vulkan Command
    //svoid Create
    //void CreateGraphicsPipeline();

protected:
    glm::vec3 mAmbientColor;
    glm::vec3 mDiffuseColor;
    glm::vec3 mSpecularColor;

    //bool mIsTwoSide                                                 = false;

    //float mShininess                                                = 0.f;
    //float mReflectivitiy                                            = 0.f;
};
