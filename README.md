# Vulkan Engine
## Precondition
* CMake
* Vulkan
* GLFW
* GLM

## Build
1. Setup Enviornment variable in CMakeEnv/xxx.cmake

Example
```CMake
set(VULKAN_SDK_PATH C:/VulkanSDK/1.3.236.0)
set(GLFW_PATH D:/ExternalLibrary/glfw-3.3.8.bin.WIN64)
set(GLM_PATH D:/ExternalLibrary/glm-0.9.9.8/glm)
```

2. Build using cmake

```CMD
cd build
cmake -S ../ -B ./
make clean
make
make Shaders
```