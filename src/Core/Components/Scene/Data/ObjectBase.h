#pragma once
#include "Core/Common/Common.h"

#include <string>

using namespace std;

class ObjectBase
{
public:
    ObjectBase(string);
    ~ObjectBase();

    string GetName() { return mName; };

protected:
    string mName;
};
