#pragma once
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <glm/glm.hpp>

namespace core
{
    class Input
    {
    public:
        static void initializeKeys();

        static void processInput(GLFWwindow* window);
        
        static bool getKey(int key);
        static bool getKeyDown(int key);
        static bool getKeyUp(int key);

        static bool getButton(int key);
        static bool getButtonDown(int key);
        static bool getButtonUp(int key);

        static glm::vec2 getMousePosition();
        static glm::vec2 getMouseDelta();
        // static void getMousePosition(double& x, double& y);
    private:
        static std::unordered_map<int, bool> prevKeys;
        static std::unordered_map<int, bool> keys;
        static std::unordered_map<int, bool> prevMouseButtons;
        static std::unordered_map<int, bool> mouseButtons;
        static glm::vec2 prevMousePosition;
        static glm::vec2 mousePosition;
    };
}