#pragma once
#include "Core/Components/Scene/Data/ObjectBase.h"
#include "Core/Components/Scene/Data/Transform.h"
#include "Core/Components/Scene/Data/MeshObject.h"
#include "Core/Components/Scene/Data/MaterialBase.h"
#include "Core/Components/Scene/Data/ShaderBase.h"

#include <string>
#include <vector>
#include <unordered_set>
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
	void SetMeshDataCallback(function<void(vector<MeshObject*>)> pCallback) { mMeshDataCallback = pCallback; };
	void SetMaterialDataCallback(function<void(vector<MaterialBase*>)> pCallback) { mMaterialDataCallback = pCallback; };
	void SetShaderDataCallback(function<void(unordered_set<ShaderType>)> pCallback) { mShaderDataCallback = pCallback; };
	void SetTransforMatrixCallback(function<void(string, glm::mat4x4)> pCallback) { mTransformMatrixCallback = pCallback; };
protected:
	// Parsing Data from loader and trigger callback
	virtual void ParseMeshsData(void** const, int) = 0;
	virtual void ParseMaterialsData(void** const, int) = 0;
	virtual void ParseTransformMatrixData(void** const, int) = 0;

	// Callback
	function<void(vector<MeshObject*>)> mMeshDataCallback;
	function<void(vector<MaterialBase*>)> mMaterialDataCallback;
	function<void(unordered_set<ShaderType>)> mShaderDataCallback;
	function<void(string, glm::mat4x4)> mTransformMatrixCallback;
};