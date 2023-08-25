#pragma once
#include <string>

using namespace std;

/// <summary>
/// This is the interface of Scene Loader
/// </summary>
class ISceneLoader
{
public:
	virtual bool LoadScene(string) = 0;
	virtual void Destroy() = 0;
private:
};