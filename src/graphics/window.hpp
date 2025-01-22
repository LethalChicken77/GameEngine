#pragma once
#include <string>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <functional>

namespace graphics
{
    class Graphics;
    class Window
    {
    public:
        Window(uint32_t width, uint32_t height, const std::string& title);
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;


        void update();
        void clear();
        void close();

        bool isOpen() const { return !glfwWindowShouldClose(window); }
        uint32_t getWidth() const { return width; }
        uint32_t getHeight() const { return height; }
        VkExtent2D getExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }
        GLFWwindow* getWindow() { return window; }
        bool windowResized() { return frameBufferResized; }
        void resetWindowResizedFlag() { frameBufferResized = false; }
        
        void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

        static void windowRefreshCallback(GLFWwindow *window);
        static void frameResizeCallback(GLFWwindow *window, int width, int height);

        bool open;

        static void setOnRefreshCallback(std::function<void()> callback) { onRefreshCallback = callback; }

    private:
        void init();
        
        uint32_t width;
        uint32_t height;
        bool frameBufferResized = false;

        static std::function<void()> onRefreshCallback;

        std::string name;
        GLFWwindow* window;
    };
}