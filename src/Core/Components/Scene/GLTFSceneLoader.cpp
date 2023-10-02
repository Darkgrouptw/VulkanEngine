#include "Core/Components/Scene/GLTFSceneLoader.h"

#pragma region Public
bool GLTFSceneLoader::LoadScene(string pPath)
{
	const aiScene* scene                                            = mImporter.ReadFile(pPath.c_str(), aiProcess_CalcTangentSpace |
                                                                                                        aiProcess_Triangulate |
                                                                                                        aiProcess_JoinIdenticalVertices |
                                                                                                        aiProcess_SortByPType);
    if (scene == NULL)
    {
        throw runtime_error("Failed to loadscene at path(" + pPath + ")");
        return false;
    }
    ConvertNode(scene);
    return true;
}
void GLTFSceneLoader::Destroy()
{
    ClearAllData();
}
#pragma endregion
#pragma region Protect
void GLTFSceneLoader::ClearAllData()
{
    #pragma region Mesh
    for (int i = 0; i < mMeshs.size(); i++)
        delete mMeshs[i];
    mMeshs.clear();
    #pragma endregion
    #pragma region Materials
	for (int i = 0; i < mMaterials.size(); i++)
		delete mMaterials[i];
    mMaterials.clear();
    #pragma endregion
}

void GLTFSceneLoader::ParseMeshsData(void** const pData, int pNumData)
{
    aiMesh** meshs                                                  = (aiMesh**)pData;
    for (int i = 0; i < pNumData; i++)
	{
		auto meshData                                               = meshs[i];
        if (meshData->mPrimitiveTypes                               != aiPrimitiveType_TRIANGLE)
            throw runtime_error("Failed to load mesh with " + to_string(meshData->mPrimitiveTypes));

        // 可能有些 Mesh 沒有 Position
		// Ex: AI_SCENE_FLAGS_ANIM_SKELETON_ONLY
        if (meshData->HasPositions())
		{
            string name(meshData->mName.data);

			MeshObject* mesh = new MeshObject(name, meshData->mMaterialIndex);
			auto vertcies = meshData->mVertices;
            auto normals = meshData->mNormals;

            bool hasNormal = meshData->HasNormals();
			for (int j = 0; j < meshData->mNumVertices; j++)
			{
                glm::vec3 pos(vertcies[j].x, vertcies[j].y, vertcies[j].z);
				if (hasNormal)
				{
	                glm::vec3 nomral(normals[j].x, normals[j].y, normals[j].z);
					mesh->InsertVertexData(pos, nomral);
			    }
				else
                    mesh->InsertVertexData(pos);
			}
            bool hasFaces = meshData->HasFaces();
            if (hasFaces)
            {
                for (int j = 0; j < meshData->mNumFaces; j++)
                    for (int k = 0; k < meshData->mFaces[j].mNumIndices; k++)
                        mesh->InsertFaceIndex(meshData->mFaces[j].mIndices[k]);
                
                if (mesh->GetFaceIndicesSize() % 3 != 0)
                    cout << "[Error] Current Mesh size is not divde by 3 :" << mesh->GetFaceIndicesSize();
            }
            else
                cout << "[Error] Current doesn't support mesh without faces" << endl;

            // 在這裡只有一個 Mesh 用一個 Material
            mesh->SetMaterialIndex(meshData->mMaterialIndex);
			mMeshs.push_back(mesh);
        }
    }
}
void GLTFSceneLoader::ParseMaterialsData(void** const pData, int pNumData)
{
	aiMaterial** materials = (aiMaterial**)pData;
    for (int i = 0; i < pNumData; i++)
    {
        auto matData                                                = materials[i];
        MaterialBase* mat                                           = new MaterialBase(string(matData->GetName().C_Str()));
        GetAllMaterialData(mat, matData);
        mMaterials.push_back(mat);
    }
}
void GLTFSceneLoader::ParseLightsData(void** const pData, int pNumData)
{
    /*aiLight** lights = (aiLight**)pData;
    for (int i = 0; i < pNumData; i++)
    {
        auto light                                                  = lights[i];
        cout << light->mColorAmbient.r << endl;
    }*/
}

void GLTFSceneLoader::GetAllMaterialData(MaterialBase* mat, aiMaterial* matData)
{
    // There are a lot of property
    // https://assimp-docs.readthedocs.io/en/latest/usage/use_the_lib.html#c-api
    aiColor3D tempColor(0.f, 0.f, 0.f);
    if (matData->Get(AI_MATKEY_COLOR_AMBIENT, tempColor) == AI_SUCCESS)
        mat->SetAmbientColor(glm::vec3(tempColor.r, tempColor.g, tempColor.b));
    if (matData->Get(AI_MATKEY_COLOR_DIFFUSE, tempColor) == AI_SUCCESS)
        mat->SetDiffuseColor(glm::vec3(tempColor.r, tempColor.g, tempColor.b));
    if (matData->Get(AI_MATKEY_COLOR_SPECULAR, tempColor) == AI_SUCCESS)
        mat->SetSpecularColor(glm::vec3(tempColor.r, tempColor.g, tempColor.b));
}
#pragma endregion
#pragma region Private
void GLTFSceneLoader::ConvertNode(const aiScene* pScene)
{
    cout << "========== Convert Scene Part ==========" << endl;
    cout << "Animation Count: "                                     << pScene->mNumAnimations << endl;
    cout << "Camera Count: "                                        << pScene->mNumCameras << endl;
    cout << "Mesh Count: "                                          << pScene->mNumMeshes << endl;
    cout << "Materials Count: "                                     << pScene->mNumMaterials << endl;
    cout << "Light Count: "                                         << pScene->mNumLights << endl;
    cout << "Skeleton Count: "                                      << pScene->mNumSkeletons << endl;
    cout << "Textures Count: "                                      << pScene->mNumTextures << endl;
    cout << endl;
    cout << "Node Children Count: "                                 << pScene->mRootNode->mNumChildren << endl;
    cout << "========== Convert Scene End ==========" << endl;

    ClearAllData();
    ParseMeshsData((void **)pScene->mMeshes,                        pScene->mNumMeshes);
    ParseMaterialsData((void**)pScene->mMaterials,                  pScene->mNumMaterials);
    ParseLightsData((void**)pScene->mLights,                        pScene->mNumLights);
}
#pragma endregion
