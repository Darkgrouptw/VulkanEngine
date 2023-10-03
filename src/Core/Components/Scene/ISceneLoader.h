#pragma once
#include "Core/Components/Scene/Data/ObjectBase.h"
#include "Core/Components/Scene/Data/Transform.h"
#include "Core/Components/Scene/Data/MeshObject.h"
#include "Core/Components/Scene/Data/MaterialBase.h"

#include <string>
#include <vector>
#include <functional>

using namespace std;

/// <summary>
/// This is the interface of Scene Loader
/// </summary>
class ISceneLoader
{
public:
	virtual bool LoadScene(string) = 0;
	virtual void Destroy() = 0;

	// 設定 Loading Finished 的 Callback
	virtual void SetMeshDataCallback(function<vector<MeshObject*>>) = 0;
	virtual void SetMaterialDataCallback(function<vector<MaterialBase*>>) = 0;
	//virtual void SetLoadingFinishedCallbak_InLightData() = 0;
protected:

	// Parsing Data from loader and trigger callback
	virtual void ParseMeshsData(void** const, int) = 0;
	virtual void ParseMaterialsData(void** const, int) = 0;
	//virtual void ParseLightsData(void** const, int) = 0;

	// Callback
	function<vector<MeshObject*>> mMeshDataCallback;
	function<vector<MaterialBase*>> mMaterialDataCallback;
};