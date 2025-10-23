cmake_minimum_required(VERSION 3.10)
project(GameEngine LANGUAGES CXX C)

# Set the compiler to MinGW
set(CMAKE_C_COMPILER "C:/msys64/ucrt64/bin/Clang.exe")
set(CMAKE_CXX_COMPILER "C:/msys64/ucrt64/bin/Clang++.exe")

message(STATUS "Compiler path: ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiler version: ${CMAKE_CXX_COMPILER_VERSION}")

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set flags
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")# -ffast-math")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
# set(ASAN_LIB_PATH "/ucrt64/lib/clang/20/lib/x86_64-w64-windows-gnu")

add_compile_options(
    -g
    # -O3
    # -ffast-math
    -fno-caret-diagnostics
    -fno-diagnostics-show-option
    -fno-diagnostics-show-note-include-stack
    -fcolor-diagnostics
)

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
include_directories("${CMAKE_SOURCE_DIR}/lib/miniz-3.0.2")
include_directories("${CMAKE_SOURCE_DIR}/lib/tinyexr-1.0.12")
include_directories("${CMAKE_SOURCE_DIR}/lib/stbimage")
include_directories("${CMAKE_SOURCE_DIR}/lib/imgui")
include_directories("${CMAKE_SOURCE_DIR}/lib/nativefiledialog-release_116/src/include")


# add_library(slang SHARED IMPORTED)
# set_target_properties(slang PROPERTIES
#     IMPORTED_LOCATION "${Vulkan_SDK_ROOT}/Bin/slang.dll"
#     INTERFACE_INCLUDE_DIRECTORIES "${Vulkan_SDK_ROOT}/Include/slang"
# )

# Add ImGui source files
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/lib/imgui)
set(IMGUI_BACKENDS ${IMGUI_DIR}/backends)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_BACKENDS}/imgui_impl_glfw.cpp
    ${IMGUI_BACKENDS}/imgui_impl_vulkan.cpp
)

set(LIB_SOURCES
    ${CMAKE_SOURCE_DIR}/lib/miniz-3.0.2/miniz.c
    ${CMAKE_SOURCE_DIR}/lib/nativefiledialog-release_116/src/nfd_common.c
    # ${CMAKE_SOURCE_DIR}/lib/nativefiledialog-release_116/src/nfd_gtk.c
    ${CMAKE_SOURCE_DIR}/lib/nativefiledialog-release_116/src/nfd_win.cpp
    # ${CMAKE_SOURCE_DIR}/lib/nativefiledialog-release_116/src/nfd_zenity.c
)

# Add all source files from ./src
file(GLOB_RECURSE SOURCES 
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_SOURCE_DIR}/src/*.c"
    "${CMAKE_SOURCE_DIR}/src/*.h"
)

# Add executable
add_executable(GameEngine ${SOURCES} ${IMGUI_SOURCES} ${LIB_SOURCES} app.rc)
target_include_directories(GameEngine PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_include_directories(GameEngine PRIVATE "${CMAKE_SOURCE_DIR}/lib/tinyobjloader")
target_include_directories(GameEngine PRIVATE "${CMAKE_SOURCE_DIR}/lib/miniz-3.0.2")
target_include_directories(GameEngine PRIVATE "${CMAKE_SOURCE_DIR}/lib/tinyexr-1.0.12")
target_include_directories(GameEngine PRIVATE "${CMAKE_SOURCE_DIR}/lib/stbimage")
target_include_directories(GameEngine PRIVATE "${CMAKE_SOURCE_DIR}/lib/imgui-1.91.8")
target_include_directories(GameEngine PRIVATE "${CMAKE_SOURCE_DIR}/lib/nativefiledialog-release_116/src/include")
target_include_directories(GameEngine PRIVATE SYSTEM "${Vulkan_INCLUDE_DIRS}/slang")

# Link libraries
target_link_libraries(GameEngine PRIVATE 
    C:/glfw-3.4.bin.WIN64/lib-mingw-w64/libglfw3.a  # Link GLFW
    Vulkan::Vulkan                                   # Link Vulkan
    # slang
    "C:/VulkanSDK/1.4.309.0/Lib/slang.lib"
    gdi32     # Windows libraries
    user32
    shell32
# if(WIN32)
    Dwmapi
# endif()
)



# # Collect shaders
# file(GLOB SHADER_SOURCES
#     ${CMAKE_SOURCE_DIR}/build/internal/shaders/*.vert
#     ${CMAKE_SOURCE_DIR}/build/internal/shaders/*.frag
# )

# file(GLOB COMPUTE_SHADER_SOURCES
#     ${CMAKE_SOURCE_DIR}/build/internal/compute_shaders/*.comp
# )

# # Output directories
# set(SHADER_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/build/internal/shaders)
# set(COMPUTE_SHADER_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/build/internal/compute_shaders)

# # Compile graphics shaders
# foreach(SHADER ${SHADER_SOURCES})
#     get_filename_component(SHADER_NAME ${SHADER} NAME)
#     add_custom_command(
#         OUTPUT ${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv
#         COMMAND glslc ${SHADER} -o ${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv
#         DEPENDS ${SHADER}
#         COMMENT "Compiling ${SHADER} to SPIR-V"
#     )
#     list(APPEND SPIRV_OUTPUTS ${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv)
# endforeach()

# # Compile compute shaders
# foreach(COMPUTE_SHADER ${COMPUTE_SHADER_SOURCES})
#     get_filename_component(SHADER_NAME ${COMPUTE_SHADER} NAME)
#     add_custom_command(
#         OUTPUT ${COMPUTE_SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv
#         COMMAND glslc ${COMPUTE_SHADER} -o ${COMPUTE_SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv
#         DEPENDS ${COMPUTE_SHADER}
#         COMMENT "Compiling ${COMPUTE_SHADER} to SPIR-V"
#     )
#     list(APPEND SPIRV_OUTPUTS ${COMPUTE_SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv)
# endforeach()

# # Add compile target
# add_custom_target(compile_shaders ALL DEPENDS ${SPIRV_OUTPUTS})
