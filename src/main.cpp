#define WINDOWS_BUILD

#include <iostream>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <memory>

#include "engine.hpp"
#include "utils/debug.hpp"
#include "graphics/window.hpp"
#include "graphics/graphics_pipeline.hpp"
#include "graphics/swap_chain.hpp"
#include "graphics/device.hpp"
#include "graphics/graphics.hpp"
#include "core/input.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef WINDOWS_BUILD
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <windows.h>
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")
#endif

const std::string APPLICATION_NAME = "Game";
const std::string ENGINE_NAME = "VEngine";

using namespace core;
using namespace graphics;

void setWindowIcons(GLFWwindow* window)
{
    // Load small icon
    int widthSmall, heightSmall, channelsSmall;
    unsigned char* pixelsSmall = stbi_load("internal/images/icon_48.png", &widthSmall, &heightSmall, &channelsSmall, 4);  // Load as RGBA
    // std::cout << "Width: " << widthSmall << " Height: " << heightSmall << std::endl;
    if (!pixelsSmall) {
        std::cerr << "Failed to load small icon!" << std::endl;
        stbi_image_free(pixelsSmall);
    }

    // Load large icon
    int widthLarge, heightLarge, channelsLarge;
    unsigned char* pixelsLarge = stbi_load("internal/images/icon_256.png", &widthLarge, &heightLarge, &channelsLarge, 4);  // Load as RGBA
    // std::cout << "Width: " << widthLarge << " Height: " << heightLarge << std::endl;
    if (!pixelsLarge) {
        std::cerr << "Failed to load large icon!" << std::endl;
        stbi_image_free(pixelsSmall);  // Free previously loaded image
        stbi_image_free(pixelsLarge);
    }

    // Create GLFWimage array for icons
    GLFWimage icons[2];
    icons[0].width = widthSmall;
    icons[0].height = heightSmall;
    icons[0].pixels = pixelsSmall;

    icons[1].width = widthLarge;
    icons[1].height = heightLarge;
    icons[1].pixels = pixelsLarge;

    // Set the window icon
    glfwSetWindowIcon(window, 2, icons);

    // Free image memory
    stbi_image_free(pixelsSmall);
    stbi_image_free(pixelsLarge);
}

#ifdef WINDOWS_BUILD
void SetTitleBarColor(GLFWwindow* window, COLORREF titleBarColor, COLORREF borderColor) {
    HWND hwnd = glfwGetWin32Window(window);
    if (hwnd) {
        std::cout << "Setting title color" << std::endl;
        // Use DwmSetWindowAttribute to set the title bar color
        const DWORD titleBarColorAttribute = DWMWA_CAPTION_COLOR;
        const DWORD borderColorAttribute = DWMWA_BORDER_COLOR;
        DwmSetWindowAttribute(hwnd, titleBarColorAttribute, &titleBarColor, sizeof(titleBarColor));
        DwmSetWindowAttribute(hwnd, borderColorAttribute, &borderColor, sizeof(borderColor));
    }
}
#endif

int main() 
{
    graphics::Graphics graphics(APPLICATION_NAME, ENGINE_NAME);

    // Set window icon
    setWindowIcons(graphics.getWindow()->getWindow());
    
#ifdef WINDOWS_BUILD
    // Set title bar color
    SetTitleBarColor(graphics.getWindow()->getWindow(), RGB(0x19, 0x15, 0x14), RGB(0x19, 0x15, 0x14));
#endif

    if(!graphics.isOpen())
    {
        std::cerr << "Failed to initialize graphics" << std::endl;
        return -1;
    }

    Engine engine(graphics);

    // glm::vec3 test = {1, 2, 3};

    engine.run();

    graphics.waitForDevice();

    return 0;
}