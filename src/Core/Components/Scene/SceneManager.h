#pragma once
#include "Core/Common/Common.h"

#if USE_ASSIMP
#include "Core/Components/Scene/GLTFSceneLoader.h"
#endif

#include <glm/gtx/matrix_decompose.hpp>

#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>

using namespace std;


class SceneManager
{
public:
	SceneManager();
	~SceneManager();

	void LoadScene(string);																					// 讀取場景
	void UpdateScene();																						// 更新資料 (Ex: Uniform Buffer)
	void RenderScene(const VkCommandBuffer);																// 開始化場景
	void UnloadScene();																						// 卸載
protected:
	// SceneData
	vector<MeshObject*> mMeshs;
	vector<MaterialBase*> mMaterials;																		// It's unique ID => like FileID, LocalID, GUID in Unity
	unordered_map<ShaderType, ShaderBase*> mShaders;

	// Callback
	void LoadedMeshDataCallback(vector<MeshObject*>);														// 整理完 Mesh 之後的 Callback
	void LoadedMaterialDataCallback(vector<MaterialBase*>);													// 整理完 Material 之後的 Callback
	void LoadedShaderDataCallback(unordered_set<ShaderType>);												// 整理完 Shader 之後的 Callback
	void LoadedTransformMatrixCallback(string, glm::mat4);													// 整理完 Tranform Matrix 的 Callback

	// Delete
	void DeleteMeshData();
	void DeleteMaterialData();
	void DeleteShaderData();

	// GPU Part
	void UploadDataToGPU();
	void DestroyGPUData();

#if USE_ASSIMP
	GLTFSceneLoader loader;
#endif
};