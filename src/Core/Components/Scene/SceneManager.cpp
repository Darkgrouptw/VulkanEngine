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

	auto meshDataCallback											= [&](vector<MeshObject*> pData) { LoadedMeshDataCallback(pData); };
	auto materialDataCallback										= [&](vector<MaterialBase*> pData) { LoadedMaterialDataCallback(pData); };
	auto shaderDataCallback											= [&](unordered_set<ShaderType> pData) { LoadedShaderDataCallback(pData); };
#if USE_ASSIMP
	loader.SetMeshDataCallback(meshDataCallback);
	loader.SetMaterialDataCallback(materialDataCallback);
	loader.SetShaderDataCallback(shaderDataCallback);
	loader.LoadScene(pSceneName);
#else
	throw runtime_error("NotImplemented other way to load scene");
#endif
	UploadDataToGPU();
}
void SceneManager::UpdateScene()
{
	// 這裡有做幾件事
	// 1. 更新全場景的物件
	// 2. 更新Uniform Buffer
}
void SceneManager::RenderScene(const VkCommandBuffer pCommandBuffer)
{
	// ToDo: 如果有空的話，以後來做 AABB 的 BVH 
	for (const auto mesh : mMeshs)
	{
		#pragma region Get all data
		int matIndex 												= mesh->GetMaterialIndex();
		MaterialBase* mat											= mMaterials[matIndex];
		ShaderType shaderType 										= mat->GetShaderType();
		ShaderBase* shader 											= mShaders[shaderType];

		VkDescriptorSet set											= shader->GetCurrentDescriptorSet();
		#pragma endregion
		#pragma region Draw Command
		shader->BindGraphicsPipeline(pCommandBuffer);
		mesh->Render(pCommandBuffer, shader->GetPipelineLayout(), set);
		#pragma endregion
	}
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
void SceneManager::LoadedShaderDataCallback(unordered_set<ShaderType> pTypes)
{
	DeleteShaderData();

	// 由於 SceneLoader
	// 只負責整理整個場景所有的 Shader Type
	// 而新增 Shader Type 是統一的動作
	// 所以在這裡執行
	for (const auto& value : pTypes)
	{
		ShaderBase* shader = new ShaderBase(value);
		mShaders.insert({value, shader});
	}
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
void SceneManager::DeleteShaderData()
{
	for(const auto& [key, value] : mShaders)
		delete value;
	mShaders.clear();
}

// GPU Data
void SceneManager::UploadDataToGPU()
{
	#pragma region Mesh
	for (const auto mesh : mMeshs)
		mesh->CreateVulkanStuff();
	#pragma endregion
	#pragma region Shader
	for (const auto& [key, shader] : mShaders)
		shader->CreateVulkanStuff();
	#pragma endregion
}
void SceneManager::DestroyGPUData()
{
	#pragma region Mesh
	for (const auto mesh : mMeshs)
		mesh->DestroyVulkanStuff();
	#pragma endregion
	#pragma region Shader
	for (const auto& [key, shader] : mShaders)
		shader->DestroyVulkanStuff();
	#pragma endregion
}
#pragma endregion
#pragma region Private
#pragma endregion
