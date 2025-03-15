cmake_minimum_required(VERSION 3.10)
project(GameEngine LANGUAGES CXX C)

# Set the compiler to MinGW
set(CMAKE_C_COMPILER "C:/msys64/ucrt64/bin/clang.exe")
set(CMAKE_CXX_COMPILER "C:/msys64/ucrt64/bin/clang++.exe")

message(STATUS "Compiler path: ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiler version: ${CMAKE_CXX_COMPILER_VERSION}")

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set flags
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
# set(ASAN_LIB_PATH "/ucrt64/lib/clang/20/lib/x86_64-w64-windows-gnu")

# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${ASAN_LIB_PATH} -lclang_rt.asan_dynamic -lclang_rt.asan_dynamic_runtime_thunk")


# Specify the output directory for the built binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_AUTORCC OFF)

# Find Vulkan
find_package(Vulkan REQUIRED)
# set(Vulkan_INCLUDE_DIRS "C:/VulkanSDK/1.4.309.0/Include")
# set(Vulkan_LIBRARIES "C:/VulkanSDK/1.4.309.0/Lib/vulkan-1.lib")

# Set GLFW Include Directory\
set(GLFW_INCLUDE_DIR "C:/glfw-3.4.bin.WIN64/include")
include_directories(${GLFW_INCLUDE_DIR})

# Include Vulkan headers
include_directories(${Vulkan_INCLUDE_DIRS})

# Include directories for your project
include_directories("${CMAKE_SOURCE_DIR}/include")
include_directories("${CMAKE_SOURCE_DIR}/lib/tinyobjloader")
include_directories("${CMAKE_SOURCE_DIR}/lib/stbimage")
include_directories("${CMAKE_SOURCE_DIR}/lib/imgui-1.91.8")

# Add ImGui source files
set(IMGUI_SOURCES
    ${CMAKE_SOURCE_DIR}/lib/imgui-1.91.8/imgui.cpp
    ${CMAKE_SOURCE_DIR}/lib/imgui-1.91.8/imgui_demo.cpp
    ${CMAKE_SOURCE_DIR}/lib/imgui-1.91.8/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/lib/imgui-1.91.8/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/lib/imgui-1.91.8/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/lib/imgui-1.91.8/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/lib/imgui-1.91.8/imgui_impl_vulkan.cpp
)

# Add all source files from ./src
file(GLOB_RECURSE SOURCES 
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_SOURCE_DIR}/src/*.c"
    "${CMAKE_SOURCE_DIR}/src/*.h"
)

# Add executable
add_executable(GameEngine ${SOURCES} ${IMGUI_SOURCES} app.rc)
target_include_directories(GameEngine PRIVATE "${CMAKE_SOURCE_DIR}/lib/tinyobjloader")
target_include_directories(GameEngine PRIVATE "${CMAKE_SOURCE_DIR}/lib/stbimage")
target_include_directories(GameEngine PRIVATE "${CMAKE_SOURCE_DIR}/lib/imgui-1.91.8")

# Link libraries
target_link_libraries(GameEngine PRIVATE 
    C:/glfw-3.4.bin.WIN64/lib-mingw-w64/libglfw3.a  # Link GLFW
    Vulkan::Vulkan                                   # Link Vulkan
    gdi32     # Windows libraries
    user32
    shell32
# if(WIN32)
    Dwmapi
# endif()
)



# Specify your shader source files.
set(SHADER_SOURCES
    ${CMAKE_SOURCE_DIR}/build/internal/shaders/basicShader.vert
    ${CMAKE_SOURCE_DIR}/build/internal/shaders/basicShader.frag
)

# Specify output directory for SPIR-V files.
set(SHADER_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/build/internal/shaders)

# Compile each shader file to SPIR-V.
foreach(SHADER ${SHADER_SOURCES})
    # Get the file name without extension (e.g., basicShader.vert -> basicShader.vert.spv).
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    add_custom_command(
        OUTPUT ${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv
        COMMAND glslc ${SHADER} -o ${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv
        DEPENDS ${SHADER}
        COMMENT "Compiling ${SHADER} to SPIR-V"
    )
    list(APPEND SPIRV_OUTPUTS ${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv)
endforeach()

# Add a custom target to compile all shaders.
add_custom_target(compile_shaders ALL DEPENDS ${SPIRV_OUTPUTS})