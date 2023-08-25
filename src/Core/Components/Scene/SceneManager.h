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
private:

#if USE_ASSIMP
	GLTFSceneLoader loader;
#endif
};