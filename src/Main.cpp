#include "Core/VulkanEngineApplication.h"

using namespace std;

int main() 
{
    VulkanEngineApplication application;
    try
    {
        application.Run();
    }
    catch (const exception &e)
    {
        cerr << e.what() << endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}