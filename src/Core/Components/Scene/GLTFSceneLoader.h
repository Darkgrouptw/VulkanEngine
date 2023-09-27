#pragma once
#if USE_ASSIMP
#include "Core/Common/Common.h"
#include "Core/Components/Scene/ISceneLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>

using namespace std;

class GLTFSceneLoader : ISceneLoader
{
public:
	virtual bool LoadScene(string) override;
	virtual void Destroy() override;

protected:
	virtual void ClearAllData() override;

	virtual void ParseMeshsData(void** const, int) override;
	virtual void ParseMaterialsData(void** const, int) override;
	virtual void ParseLightsData(void** const, int) override;


	void GetAllMaterialData(MaterialBase*, aiMaterial* mat);
private:
	Assimp::Importer mImporter;

	void ConvertNode(const aiScene*);
	
};
#endif