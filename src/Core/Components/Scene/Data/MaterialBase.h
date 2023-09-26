#pragma once
#include "Core/Common/Common.h"
#include "Core/Components/Scene/Data/ObjectBase.h"
#include "Core/Components/Scene/Data/Transform.h"

class MaterialBase : public ObjectBase
{
public:
    MaterialBase(string);
    ~MaterialBase();
};
