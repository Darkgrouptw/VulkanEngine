#include "Core/Components/Scene/SceneManager.h"

#pragma region Public
SceneManager::SceneManager()
{
	mMainCamera = new Camera("Main Camera");
}
SceneManager::~SceneManager()
{
	DeleteMeshData();
	DeleteMaterialData();

	delete mMainCamera;
}

void SceneManager::LoadScene(string pSceneName)
{
	pSceneName = Common::GetResourcePath(pSceneName);

	auto meshDataCallback											= [&](vector<MeshObject*> pData) { LoadedMeshDataCallback(pData); };
	auto materialDataCallback										= [&](vector<MaterialBase*> pData) { LoadedMaterialDataCallback(pData); };
	auto shaderDataCallback											= [&](unordered_set<ShaderType> pData) { LoadedShaderDataCallback(pData); };
	auto tranformMatrixCallback										= [&](string pName, glm::mat4 pMatrix) { LoadedTransformMatrixCallback(pName, pMatrix); }; 
#if USE_ASSIMP
	loader.SetMeshDataCallback(meshDataCallback);
	loader.SetMaterialDataCallback(materialDataCallback);
	loader.SetShaderDataCallback(shaderDataCallback);
	loader.SetTransforMatrixCallback(tranformMatrixCallback);
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
		glm::mat4 projM 											= mMainCamera->GetProjectMatrix();
		glm::mat4 viewM												= mMainCamera->GetViewMatrix();
		glm::mat4 modelM 											= mesh->GetModelMatrix();

		/*static auto startTime											= chrono::high_resolution_clock::now();
		auto currentTime												= chrono::high_resolution_clock::now();
		float duration													= chrono::duration<float, chrono::seconds::period>(currentTime - startTime).count();*/
		shader->SetMVPUniformBuffer(projM, viewM, modelM);
		if (shaderType == ShaderType::PBR)
		{
			PBRShader* pbrShader									= (PBRShader*)shader;
			pbrShader->SetMatUniformBuffer(mat->GetAmbientColor(), mat->GetDiffuseColor(), mat->GetSpecularColor());
		}


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
		ShaderBase* shader = ShaderTypeUtils::GenerateShaderFactory(value);
		mShaders.insert({value, shader});
	}
}
void SceneManager::LoadedTransformMatrixCallback(string pName, glm::mat4 pMatrix)
{
	// Parse every data if it is the name
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;
	
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(pMatrix, scale, rotation, position, skew, perspective);
	rotation = glm::conjugate(rotation);

	#pragma region Mesh
	for (const auto mesh : mMeshs)
		if (mesh->GetName() == pName)
		{
			mesh->Position = position;
			mesh->Rotation = rotation;
			mesh->Scale = scale;
			return;
		}
	#pragma endregion
	#pragma region Camera
	// ToDo: Add multi camera
	/*if (pName.find("Camera") != string::npos)
	{
		cout << "Main Camera(" << pName << ") position: " << position.x << " " << position.y << " " << position.z << endl;
		cout << "Main Camera(" << pName << ") rotation: " << rotation.x << " " << rotation.y << " " << rotation.z << " " << rotation.w << endl;
		mMainCamera->Position = position;
		mMainCamera->Rotation = rotation;
		return;
	}*/
	#pragma endregion
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