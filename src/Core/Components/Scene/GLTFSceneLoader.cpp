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
}

void GLTFSceneLoader::ParseMeshs(void** const pData, int pNumData)
{
    aiMesh** meshs = (aiMesh**)pData;
    for (int i = 0; i < pNumData; i++)
	{
        if (meshs[i]->mPrimitiveTypes                               != aiPrimitiveType_TRIANGLE)
            throw runtime_error("Failed to load mesh with " + to_string(meshs[i]->mPrimitiveTypes));

        // 可能有些 Mesh 沒有 Position
		// Ex: AI_SCENE_FLAGS_ANIM_SKELETON_ONLY
        if (meshs[i]->HasPositions())
		{
			MeshObject* mesh = new MeshObject();
			auto vertcies = meshs[i]->mVertices;
            auto normals = meshs[i]->mNormals;

            bool hasNormal = meshs[i]->HasNormals();
			for (int j = 0; j < meshs[i]->mNumVertices; j++)
			{
                glm::vec3 pos(vertcies[j].x, vertcies[j].y, vertcies[j].z);
				if (hasNormal)
				{
	                glm::vec2 nomral(normals[j].x, normals[j].y);
					mesh->InsertPositionAndNormal(pos, nomral);
			    }
				else
                    mesh->InsertPosition(pos);
			}
			mMeshs.push_back(mesh);
        }
    }
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
    cout << "========== Convert Scene End ==========" << endl;

    ClearAllData();
    ParseMeshs((void **)pScene->mMeshes, pScene->mNumMeshes);
    //if ()
    //ParseMe
    /*pragma region Mesh
    auto meshs                                                      = pScene->mMeshes;
    for(int i = 0; i < pScene->mNumMeshes; i++)
    {
        if (meshs[i]->mPrimitiveTypes                               != aiPrimitiveType_TRIANGLE)
            throw runtime_error("Not implement in other type");


        for (int j = 0; j < meshs[i]->mNumVertices; j++)
        {
            glm::vec3 pos = glm::vec3(meshs[i]->mVertices[j].x, meshs[i]->mVertices[j].y, meshs[i]->mVertices[j].z);
        }
			 
        //cout << meshs[i]->HasPositions() << " " << meshs[i]->HasNormals() << endl;
    }
    #pragma endregion*/
}
#pragma endregion
