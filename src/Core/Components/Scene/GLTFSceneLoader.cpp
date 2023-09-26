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
    for (auto it = mMaterials.begin(); it != mMaterials.end(); it++)
        delete it->second;
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
					mesh->InsertPositionAndNormal(pos, nomral);
			    }
				else
                    mesh->InsertPosition(pos);
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
        for (int j = 0; j < matData->mNumProperties; j++)
        {
            auto prop                                               = matData->mProperties[j];
            //pro
            cout << prop->mDataLength << endl;
        }
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
