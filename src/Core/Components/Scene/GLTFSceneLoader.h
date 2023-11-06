#pragma once
#if USE_ASSIMP
#include "Core/Common/Common.h"
#include "Core/Components/Scene/ISceneLoader.h"
#include "Core/Components/Scene/Data/GLTFShaderTypeUtils.hpp"

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
	bool LoadScene(string) override;
	void Destroy() override;

protected:
	void ParseMeshsData(void** const, int) override;
	void ParseMaterialsData(void** const, int) override;
	void ParseTransformMatrixData(void** const, int) override;

	ShaderType GetAllGLTFMaterialData(MaterialBase*, aiMaterial* mat);
private:
	Assimp::Importer mImporter;

	void ConvertNode(const aiScene*);
};
#endif