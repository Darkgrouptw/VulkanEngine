# 1. Include the dependencies on Env
if (WIN32)
    include("./CMakeEnv/WindowEnv.cmake" RESULT_VARIABLE LOCAL_ENV)
elseif(APPLE)
    include("./CMakeEnv/MacEnv.cmake" RESULT_VARIABLE LOCAL_ENV)
else()
    message(FATAL_ERROR "It's not tested on Linux yet")
endif()
message(STATUS "Using include: ${LOCAL_ENV}")

# 2. Setup some detail
cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(VulkanEngine)
set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME} src/Main.cpp)


# 3. Setup Library Stuff
# 3a. Vulkan
if (DEFINED VULKAN_SDK_PATH)
    set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include")
    set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib")
    set(Vulkan_FOUND "True")
else()
    find_package(Vulkan REQUIRED)
    message(STATUS "Found Vulkan Lib: ${Vulkan_LIBRARIES}")
endif()
if (NOT Vulkan_FOUND)
    message(FATAL ERROR "Not Found Vulkan")
else()
    message(STATUS "Using Vulkan Lib: ${Vulkan_LIBRARIES}")
endif()

# 3d. Final
target_include_directories(${PROJECT_NAME} PUBLIC
    ${Vulkan_INCLUDE_DIRS}
    # ${TINYOBJ_PATH}
    # ${GLFW_INCLUDE_DIRS}
    # ${GLM_PATH}
)
target_link_directories(${PROJECT_NAME} PUBLIC
    ${Vulkan_LIBRARIES}
    # ${GLFW_LIB}
)