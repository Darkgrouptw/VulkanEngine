#pragma once
#include "Core/Components/Scene/Data/Shader/ShaderBase.h"

class UnlitShader : public ShaderBase
{
public:
	UnlitShader();
	~UnlitShader();

protected:
    ShaderType GetShaderType() override;
};