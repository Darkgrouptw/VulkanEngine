#include "Core/Components/Scene/SceneManager.h"

#pragma region Public
SceneManager::SceneManager(VkDevice& pDevice)
{
	mDevice = pDevice;
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

}
void SceneManager::DestroyGPUData()
{
	// ToDo: Add When init buffer
	/*vkDestroyBuffer(Device, IndexBuffer, nullptr);
	vkFreeMemory(Device, IndexBufferMemory, nullptr);

	vkDestroyBuffer(Device, VertexBuffer, nullptr);
	vkFreeMemory(Device, VertexBufferMemory, nullptr);*/
}
#pragma endregion
#pragma region Private
#pragma endregion
