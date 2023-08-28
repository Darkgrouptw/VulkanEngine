#include "Core/Components/Scene/SceneManager.h"

#pragma region Public
SceneManager::SceneManager()
{

}
SceneManager::~SceneManager()
{

}

void SceneManager::LoadScene(string pSceneName)
{
	pSceneName = Common::GetResourcePath(pSceneName);
#if USE_ASSIMP
	loader.LoadScene(pSceneName);
#else
	throw runtime_error("NotImplemented other way to load scene");
#endif
}
#pragma endregion
#pragma region Private
#pragma endregion
