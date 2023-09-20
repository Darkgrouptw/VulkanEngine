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

	virtual void ParseMeshs(void** const, int) override;
	virtual void ParseMaterials(void** const, int) override;

private:
	Assimp::Importer mImporter;

	void ConvertNode(const aiScene*);
	
};
#endif