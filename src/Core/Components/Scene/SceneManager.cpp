#include "Core/Components/Scene/SceneManager.h"

#pragma region Public
SceneManager::SceneManager()
{
}
SceneManager::~SceneManager()
{
	DeleteMeshData();
	DeleteMaterialData();
}

void SceneManager::LoadScene(string pSceneName)
{
	pSceneName = Common::GetResourcePath(pSceneName);

	auto pMeshDataCallback											= [&](vector<MeshObject*> pData) { LoadedMeshDataCallback(pData); };
	auto pMaterialDataCallback										= [&](vector<MaterialBase*> pData) { LoadedMaterialDataCallback(pData);};
#if USE_ASSIMP
	loader.SetMeshDataCallback(pMeshDataCallback);
	//loader.SetMaterialDataCallback(pMaterialDataCallback);
	loader.LoadScene(pSceneName);
#else
	throw runtime_error("NotImplemented other way to load scene");
#endif
	UploadDataToGPU();
}
void SceneManager::UnloadScene()
{
	DestroyGPUData();
}

#pragma endregion
#pragma region Protected
// Callback
void SceneManager::LoadedMeshDataCallback(vector<MeshObject*> pData)
{
	DeleteMeshData();
	mMeshs = pData;
}
void SceneManager::LoadedMaterialDataCallback(vector<MaterialBase*> pData)
{
	DeleteMaterialData();
	mMaterials = pData;
}

// Delete
void SceneManager::DeleteMeshData()
{
	for (int i = 0; i < mMeshs.size(); i++)
		delete mMeshs[i];
	mMeshs.clear();
}
void SceneManager::DeleteMaterialData()
{
	for (int i = 0; i < mMaterials.size(); i++)
		delete mMaterials[i];
	mMaterials.clear();
}

// GPU Data
void SceneManager::UploadDataToGPU()
{
	#pragma region Mesh
	for (int i = 0; i < mMeshs.size(); i++)
	{
		mMeshs[i]->CreateVertexBuffer();
		//mMeshs[i]->CreateIndicesBuffer();
	}
	#pragma endregion
}
void SceneManager::DestroyGPUData()
{
	#pragma region Mesh
	for (int i = 0; i < mMeshs.size(); i++)
		mMeshs[i]->DestroyVertexBuffer();
	#pragma endregion
}
#pragma endregion
#pragma region Private
#pragma endregion
