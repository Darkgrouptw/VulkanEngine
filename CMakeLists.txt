cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(VulkanEngine)

add_executable(${PROJECT_NAME} src/Main.cpp)

# Vulkan
find_package(Vulkan REQUIRED)
if (Vulkan_FOUND)
    include_directories(${Vulkan_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})
else()
    message(FATAL_ERROR "No Vulkan Found")
endif()
# message ("Found Vulkan Include: " + ${Vulkan_INCLUDE_DIRS})
# message ("Found Vulkan Library: " + ${Vulkan_LIBRARIES})

# GLFW3
find_package(glfw3 REQUIRED)
if (glfw3_FOUND)
    message (${GLFW3_INCLUDE_DIRS}) 
    include_directories(${GLFW3_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${GLFW3_LIBRARIES})
else()
    message (FATAL_ERROR "No GLFW3 Found") 
endif()

# GLM
find_package(glm REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${GLM_LIBRARIES})

set(CMAKE_CXX_STANDARD 20)