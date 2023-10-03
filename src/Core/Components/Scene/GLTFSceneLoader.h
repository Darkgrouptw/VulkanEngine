#pragma once
#if USE_ASSIMP
#include "Core/Common/Common.h"
#include "Core/Components/Scene/ISceneLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>

using namespace std;

/// <summary>
/// 用來載入 GLTF 的 Loader
/// </summary>
class GLTFSceneLoader : public ISceneLoader
{
public:
	virtual bool LoadScene(string) override;
	virtual void Destroy() override;

protected:
	virtual void ParseMeshsData(void** const, int) override;
	virtual void ParseMaterialsData(void** const, int) override;
	//virtual void ParseLightsData(void** const, int) override;

	void GetAllGLTFMaterialData(MaterialBase*, aiMaterial* mat);
private:
	Assimp::Importer mImporter;

	void ConvertNode(const aiScene*);
};
#endif