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