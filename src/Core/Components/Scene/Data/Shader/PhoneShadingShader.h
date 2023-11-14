#pragma once
#include "Core/Components/Scene/Data/Shader/ShaderBase.h"

class PhoneShadingShader : public ShaderBase
{
public:
	PhoneShadingShader();
	~PhoneShadingShader();

protected:
	ShaderType GetShaderType() override;
};