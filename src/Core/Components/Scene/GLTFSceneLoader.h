#pragma once
#if USE_ASSIMP
#include "Core/Common/Common.h"
#include "Core/Components/Scene/ISceneLoader.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

class GLTFSceneLoader : ISceneLoader
{
public:
	virtual bool LoadScene(string) override;
	virtual void Destroy() override;
	aiScene* GetScene();
private:
	Assimp::Importer mImporter;
	aiScene* mScene;
};
#endif