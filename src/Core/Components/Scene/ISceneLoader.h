#pragma once
#include "Core/Components/Scene/Data/ObjectBase.h"
#include "Core/Components/Scene/Data/Transform.h"
#include "Core/Components/Scene/Data/MeshObject.h"
#include "Core/Components/Scene/Data/MaterialBase.h"

#include <string>
#include <vector>
#include <unordered_map>

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
	virtual void ParseMeshsData(void** const, int) = 0;
	virtual void ParseMaterialsData(void** const, int) = 0;
	virtual void ParseLightsData(void** const, int) = 0;

	vector<MeshObject* > mMeshs;
	unordered_map<int, MaterialBase*> mMaterials;														// It's unique ID => like FileID, LocalID, GUID in Unity 
};