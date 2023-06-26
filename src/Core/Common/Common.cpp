#include "Common.h"

string Common::GetResourcePath(string path)
{
    // cout << filesystem::current_path() << endl;
    if (filesystem::is_directory("Resources"))
    {
    }
        cout << filesystem::path("Resources/" + path).filename() << endl;
    return "";
}