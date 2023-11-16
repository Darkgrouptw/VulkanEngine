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
    mMeshDataCallback = NULL;
    mMaterialDataCallback = NULL;
}
#pragma endregion
#pragma region Protected
void GLTFSceneLoader::ParseMeshsData(void** const pData, int pNumData)
{
    vector<MeshObject*> meshList;
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
			auto vertcies                                           = meshData->mVertices;
            auto normals                                            = meshData->mNormals;
            auto vcolors                                            = meshData->mColors;
            auto uvs                                                = meshData->mTextureCoords;

            
            // Currently support set 0
            bool hasNormal                                          = meshData->HasNormals();
            bool hasVertexColor                                     = meshData->HasVertexColors(0);
            bool hasTexcoords                                       = meshData->HasTextureCoords(0);
			for (int j = 0; j < meshData->mNumVertices; j++)
			{
                glm::vec3 pos(vertcies[j].x, vertcies[j].y, vertcies[j].z);
                glm::vec3 normal(0.f, 0.f, 0.f);
                glm::vec4 vcolor(0.f, 0.f, 0.f, 0.f);
                glm::vec2 uv(0.f, 0.f);

                if (hasNormal)
                    normal                                          = glm::vec3((float)normals[j].x, (float)normals[j].y, (float)normals[j].z);
                if (hasVertexColor)
                    vcolor                                          = glm::vec4((float)vcolors[0][j].r, (float)vcolors[0][j].g, (float)vcolors[0][j].b, (float)vcolors[0][j].a);
                if (hasTexcoords)
                    uv                                              = glm::vec2((float)uvs[0][j].x, (float)uvs[0][j].y);
                mesh->InsertVertexData(pos, normal, vcolor, uv);
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
			meshList.push_back(mesh);
        }
    }
    if (mMeshDataCallback != NULL)
        mMeshDataCallback(meshList);
}
void GLTFSceneLoader::ParseMaterialsData(void** const pData, int pNumData)
{
    vector<MaterialBase*> materialList;
    unordered_set<ShaderType> shaderList;
	aiMaterial** materials                                          = (aiMaterial**)pData;
    for (int i = 0; i < pNumData; i++)
    {
        auto matData                                                = materials[i];
        MaterialBase* mat                                           = new MaterialBase(string(matData->GetName().C_Str()));
        ShaderType shaderType                                       = GetAllGLTFMaterialData(mat, matData);

        materialList.push_back(mat);
        if (!shaderList.contains(shaderType))
            shaderList.insert(shaderType);
	}
	if (mMaterialDataCallback != NULL)
        mMaterialDataCallback(materialList);
    if (mShaderDataCallback != NULL)
        mShaderDataCallback(shaderList);
}
void GLTFSceneLoader::ParseTransformMatrixData(void** const pData, int pNumData)
{
    aiNode** nodes                                                  = (aiNode**)pData;
    for(int i = 0 ; i < pNumData; i++)
    {
        auto matrix                                                 = nodes[i]->mTransformation;
        glm::mat4 toMatrix;

        // glm is column major, Assimp matrix is row major
        toMatrix[0][0] = matrix.a1; toMatrix[1][0] = matrix.a2; toMatrix[2][0] = matrix.a3; toMatrix[3][0] = matrix.a4;
        toMatrix[0][1] = matrix.b1; toMatrix[1][1] = matrix.b2; toMatrix[2][1] = matrix.b3; toMatrix[3][1] = matrix.b4;
        toMatrix[0][2] = matrix.c1; toMatrix[1][2] = matrix.c2; toMatrix[2][2] = matrix.c3; toMatrix[3][2] = matrix.c4;
        toMatrix[0][3] = matrix.d1; toMatrix[1][3] = matrix.d2; toMatrix[2][3] = matrix.d3; toMatrix[3][3] = matrix.d4;

        // send to callback
        mTransformMatrixCallback(string(nodes[i]->mName.data), toMatrix);
    }
}

ShaderType GLTFSceneLoader::GetAllGLTFMaterialData(MaterialBase* mat, aiMaterial* matData)
{
    // There are a lot of property
    // https://assimp-docs.readthedocs.io/en/latest/usage/use_the_lib.html#c-api
    aiColor4D tempColor(0.f, 0.f, 0.f, 0.f);
    if (matData->Get(AI_MATKEY_COLOR_AMBIENT, tempColor) == AI_SUCCESS)
        mat->SetAmbientColor(glm::vec4(tempColor.r, tempColor.g, tempColor.b, tempColor.a));
    if (matData->Get(AI_MATKEY_COLOR_DIFFUSE, tempColor) == AI_SUCCESS)
        mat->SetDiffuseColor(glm::vec4(tempColor.r, tempColor.g, tempColor.b, tempColor.a));
    if (matData->Get(AI_MATKEY_COLOR_SPECULAR, tempColor) == AI_SUCCESS)
        mat->SetSpecularColor(glm::vec4(tempColor.r, tempColor.g, tempColor.b, tempColor.a));

    // Get Shader Model
    aiShadingMode mode = aiShadingMode::aiShadingMode_Unlit;
    matData->Get(AI_MATKEY_SHADING_MODEL, mode);

    // Debug Material Property
    /*for (int i = 0; i < matData->mNumProperties; i++)
    {
        auto prop = matData->mProperties[i];
        cout << "Material prop: " << prop->mKey.C_Str() << " value: " << (int)(*(prop->mData)) << " length: " << prop->mDataLength  << endl;
    }*/
    
    // Transfor to Shader Type
    ShaderType type = GLTFShaderTypeUtils::GetShaderTypeFrom(mode);
    mat->SetShaderType(type);
    return type;
}
#pragma endregion
#pragma region Private
void GLTFSceneLoader::ConvertNode(const aiScene* pScene)
{
    auto* rootNode                                                  = pScene->mRootNode;

    cout << "========== Convert Scene Part ==========" << endl;
    cout << "Animation Count: "                                     << pScene->mNumAnimations << endl;
    cout << "Camera Count: "                                        << pScene->mNumCameras << endl;
    cout << "Mesh Count: "                                          << pScene->mNumMeshes << endl;
    cout << "Materials Count: "                                     << pScene->mNumMaterials << endl;
    cout << "Light Count: "                                         << pScene->mNumLights << endl;
    cout << "Skeleton Count: "                                      << pScene->mNumSkeletons << endl;
    cout << "Textures Count: "                                      << pScene->mNumTextures << endl;
    cout << endl;
    cout << "Node Children Count: "                                 << rootNode->mNumChildren << endl;
    cout << "========== Convert Scene End ==========" << endl;

    ParseMeshsData((void **)pScene->mMeshes,                        pScene->mNumMeshes);
    ParseMaterialsData((void**)pScene->mMaterials,                  pScene->mNumMaterials);
    ParseTransformMatrixData((void**)rootNode->mChildren,           rootNode->mNumChildren);
}
#pragma endregion
