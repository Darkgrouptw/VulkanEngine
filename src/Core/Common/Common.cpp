#include "Common.h"

string Common::GetResourcePath(string pFilePath)
{
    // 檢查的列表
    vector<string> checkList{
        "Resources",
        "../Resources"
    };

    string path = pFilePath;
    for(int i = 0; i < checkList.size(); i++) 
        if (filesystem::is_directory(checkList[i]))
        {
            path = filesystem::path(checkList[i] + "/" + pFilePath).generic_string();
            break;
        }
    
    if (!filesystem::exists(path))
        throw runtime_error("File don't exist: " + path);
    return path;
}
string Common::GetShaderPath(string pFilePath)
{
	// 檢查是否存在 build 資料夾內
	// 1. 如果是 不須增加 "./build/"
	// 2. 如果不是 增加 "./build/"
	string currentPath = filesystem::current_path().string();
	string buildPath = "build";
	bool IsInBuildDir = false;
	if (currentPath.length() >= buildPath.length())
		IsInBuildDir = currentPath.compare(currentPath.length() - buildPath.length(), buildPath.length(), buildPath) == 0; // 如果結尾是 build 輸出 0

    string dirPath = "./build/Shaders/";
    if (IsInBuildDir)
        dirPath = "Shaders/";
    return dirPath + pFilePath;
}