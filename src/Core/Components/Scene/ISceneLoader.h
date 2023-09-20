#pragma once
#include "Core/Components/Scene/Data/ObjectBase.h"
#include "Core/Components/Scene/Data/MeshObject.h"

#include <string>
#include <vector>

using namespace std;

/// <summary>
/// This is the interface of Scene Loader
/// </summary>
class ISceneLoader
{
public:
	virtual bool LoadScene(string) = 0;
	virtual void Destroy() = 0;

protected:
	virtual void ClearAllData() = 0;

	// Parsing Data from loader
	virtual void ParseMeshs(void** const, int) = 0;
	virtual void ParseMaterials(void** const, int) = 0;

	vector<MeshObject* > mMeshs;
};