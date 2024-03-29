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

# 3. Setup Library Stuff
# 3a. Vulkan
if (DEFINED VULKAN_SDK_PATH)
    set(Vulkan_INCLUDE_DIR "${VULKAN_SDK_PATH}/Include")
    set(Vulkan_LIBRARY "${VULKAN_SDK_PATH}/Lib")
    set(Vulkan_FOUND "True")
else()
    find_package(Vulkan REQUIRED)
    message(STATUS "Find Vulkan Package")
endif()
if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Not Found Vulkan")
else()
    message(STATUS "Using Vulkan Include: ${Vulkan_INCLUDE_DIR}")
    message(STATUS "Using Vulkan Library: ${Vulkan_LIBRARY}")
endif()

# 3b. GLFW
if (DEFINED GLFW_PATH)
    set(GLFW_INCLUDE_DIR "${GLFW_PATH}/Include")
    if (WIN32)
        set(GLFW_LIBRARY "${GLFW_PATH}/lib-vc2022") # 根據下載的版本 Visual Studio 2022
    else()
        set(GLFW_LIBRARY "${GLFW_PATH}/lib") # 其他平台應該都是在這個下面
    endif()
else()
    find_package(glfw3 3.3 REQUIRED)
    message(STATUS "Found GLFW Lib: ${GLFW_LIBRARY}")
endif()
if (NOT DEFINED GLFW_LIBRARY)
    message(FATAL_ERROR "Not Found GLFW")
else()
    message(STATUS "Using GLFW Lib: ${GLFW_LIBRARY}")
endif()

# 3c. IMGUI
set (IMGUI_INCLUDE_DIR "Plugins/imgui")
set (IMGUI_CPP_FILE "Plugins/imgui/imgui.cpp")
list(APPEND IMGUI_CPP_FILE "Plugins/imgui/backends/imgui_impl_glfw.cpp")
list(APPEND IMGUI_CPP_FILE "Plugins/imgui/backends/imgui_impl_vulkan.cpp")
list(APPEND IMGUI_CPP_FILE "Plugins/imgui/imgui_draw.cpp")
list(APPEND IMGUI_CPP_FILE "Plugins/imgui/imgui_widgets.cpp")
list(APPEND IMGUI_CPP_FILE "Plugins/imgui/imgui_tables.cpp")

# 3d. Assimp
if (DEFINED Assimp_SDK_PATH)
    set(Assimp_INCLUDE_DIR "${Assimp_SDK_PATH}/include")
    if (WIN32)
        set(Assimp_LIBRARY "${Assimp_SDK_PATH}/lib/x64")
    else()
        set(Assimp_LIBRARY "${Assimp_SDK_PATH}/lib")
    endif()
    message(STATUS "Using Assimp Include Dir: ${Assimp_INCLUDE_DIR}")
    message(STATUS "Using Assimp Library Dir: ${Assimp_LIBRARY}")
else()
    message(FATAL_ERROR "Not found Assimp_SDK_PATH")
endif()

# 3e. Magic Enum
set (MAGIC_ENUM_INCLUDE_DIR "Plugins/magic_enum/include")

# 4. Final
file(GLOB_RECURSE SOURCE_FILES
    "src/*.h"
    "src/*.cpp"
    "src/*.hpp"
)

# message("All Files: ${SOURCE_FILES}")
add_executable(${PROJECT_NAME}
    ${SOURCE_FILES}
    ${IMGUI_CPP_FILE}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${Vulkan_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIR}
    ${IMGUI_INCLUDE_DIR}
    ${Assimp_INCLUDE_DIR}
    ${MAGIC_ENUM_INCLUDE_DIR}
    "./Plugins"
    "./src"
)

if (WIN32)
    message(STATUS "Building For Windows")
    target_link_directories(${PROJECT_NAME} PRIVATE
        ${Vulkan_LIBRARY}
        ${GLFW_LIBRARY}
        ${Assimp_LIBRARY}
    )
    target_link_libraries(${PROJECT_NAME} 
        glfw3
        vulkan-1
        assimp-vc143-mt
    )
else()
    message(STATUS "Building For Unix")
    target_link_libraries(${PROJECT_NAME} 
        ${Vulkan_LIBRARY}
        ${GLFW_LIBRARY}/libglfw.dylib
        ${Assimp_LIBRARY}/libassimp.dylib
    )
endif()

# 5. Shader (Use glslc to compile to SPRIV)
# Refernce: 
# https://github.com/blurrypiano/littleVulkanEngine/blob/main/CMakeLists.txt
# https://github.com/google/shaderc/tree/main/glslc#311-shader-stage-specification
find_program(GLSLC glslc HINTS
    /usr/bin 
    /usr/local/bin 
    ${VULKAN_SDK_PATH}/Bin
    ${VULKAN_SDK_PATH}/Bin32
    $ENV{VULKAN_SDK}/Bin/ 
    $ENV{VULKAN_SDK}/Bin32/
)
# message(${GLSLC})
file(GLOB_RECURSE SHADER_SOURCE_FILES
     "Shaders/*.vert"
     "Shaders/*.tesc"
     "Shaders/*.tese"
     "Shaders/*.geom"
     "Shaders/*.frag"
     "Shaders/*.comp"
)
if (NOT EXISTS "${PROJECT_SOURCE_DIR}/build/Shaders")
    make_directory("${PROJECT_SOURCE_DIR}/build/Shaders")
endif()

foreach(FILE ${SHADER_SOURCE_FILES})
    get_filename_component(FILENAME ${FILE} NAME)
    set(SPRIV_OUTPUT_FILE "${PROJECT_SOURCE_DIR}/build/Shaders/${FILENAME}.spv")
    
    add_custom_command(
        OUTPUT ${SPRIV_OUTPUT_FILE}
        COMMAND ${GLSLC} ${FILE} -o ${SPRIV_OUTPUT_FILE}
        DEPENDS ${GLSLC}
    )
    list(APPEND SPIRV_BINARY_FILES ${SPRIV_OUTPUT_FILE})
endforeach(FILE)

# add to target
add_custom_target(
    Shaders
    DEPENDS ${SPIRV_BINARY_FILES}
)

# add define
add_compile_definitions(
    USE_ASSIMP
)