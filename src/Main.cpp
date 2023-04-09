#include <iostream>

#include "Core/Vulkan/Application.h"

using namespace std;

int main() 
{
    VulkanEngine::Application application;
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