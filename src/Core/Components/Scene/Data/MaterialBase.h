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

    //void InsertProp(string, )
protected:
    unordered_map<string, string> mProperties;
};
