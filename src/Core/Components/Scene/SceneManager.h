#pragma once
#include "Core/Common/Common.h"

#if USE_ASSIMP
#include "Core/Components/Scene/GLTFSceneLoader.h"
#endif

#include <iostream>
#include <string>

using namespace std;


class SceneManager
{
public:
	SceneManager();
	~SceneManager();

	void LoadScene(string);
protected:
	// SceneData
	vector<MeshObject* > mMeshs;
	vector<MaterialBase*> mMaterials;																		// It's unique ID => like FileID, LocalID, GUID in Unity 


#if USE_ASSIMP
	GLTFSceneLoader loader;
#endif
};