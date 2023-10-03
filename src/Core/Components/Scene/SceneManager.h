#pragma once
#include "Core/Common/Common.h"

#if USE_ASSIMP
#include "Core/Components/Scene/GLTFSceneLoader.h"
#endif

#include <iostream>
#include <string>
#include <functional>

using namespace std;


class SceneManager
{
public:
	SceneManager(VkDevice&);
	~SceneManager();

	void LoadScene(string);
protected:
	// SceneData
	vector<MeshObject*> mMeshs;
	vector<MaterialBase*> mMaterials;																		// It's unique ID => like FileID, LocalID, GUID in Unity 

	// Callback
	void LoadedMeshDataCallback(vector<MeshObject*>);														// 載完 Mesh 之後的 Callback
	void LoadedMaterialDataCallback(vector<MaterialBase*>);													// 載完 Material 之後的 Callback

	// Delete
	void DeleteMeshData();
	void DeleteMaterialData();

	// GPU Part
	void UploadDataToGPU();
	void DestroyGPUData();
	VkDevice mDevice;

#if USE_ASSIMP
	GLTFSceneLoader loader;
#endif
};