#pragma once
#include "Core/Common/Common.h"
#include "Core/Components/Scene/Data/GLTFShaderTypeUtils.hpp"
#include "Core/Components/Scene/Data/ObjectBase.h"

using namespace std;

class ShaderBase : public ObjectBase
{
public:
    ShaderBase(const ShaderType);
    ~ShaderBase();

	// Vulkan Command
	void CreateDescriptorSetLayout();                                                                       // 在建立 GraphicsPipeline 前，要設定好 Uniform Buffer 的設定
	void CreateGraphicsPipeline();                                                                          // 建立 Graphics Pipeline

protected:
    ShaderType mType;

    glm::vec3 mAmbientColor;
    glm::vec3 mDiffuseColor;
    glm::vec3 mSpecularColor;

    //bool mIsTwoSide                                                 = false;

    //float mShininess                                                = 0.f;
    //float mReflectivitiy                                            = 0.f;

    vector<char> ReadShaderFile(const string&);

    // Vulkan Pipeline
    VkDescriptorSetLayout mDescriptorSetLayout;
};
