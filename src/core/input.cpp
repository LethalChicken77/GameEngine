#include "input.hpp"
#include <GLFW/glfw3.h>
#include <iostream>

namespace core
{

std::unordered_map<int, bool> Input::prevKeys{};
std::unordered_map<int, bool> Input::keys{};
std::unordered_map<int, bool> Input::prevMouseButtons{};
std::unordered_map<int, bool> Input::mouseButtons{};
glm::vec2 Input::prevMousePosition{};
glm::vec2 Input::mousePosition{};

void Input::initializeKeys() 
{
    for (int i = 0; i < GLFW_KEY_LAST; i++) 
    {
        prevKeys[i] = false;
        keys[i] = false;
    }
    for (int i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++) 
    {
        prevMouseButtons[i] = false;
        mouseButtons[i] = false;
    }
}

void Input::processInput(GLFWwindow* window)
{
    for(int i = 0; i < GLFW_KEY_LAST; i++)
    {
        prevKeys[i] = keys[i];
        keys[i] = glfwGetKey(window, i) == GLFW_PRESS;
    }
    for(int i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++)
    {
        prevMouseButtons[i] = mouseButtons[i];
        mouseButtons[i] = glfwGetMouseButton(window, i) == GLFW_PRESS;
    }
    
    prevMousePosition = mousePosition;
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    mousePosition = glm::vec2(mouseX, mouseY);
}

bool Input::getKey(int key)
{
    return keys[key];
}

bool Input::getKeyDown(int key)
{
    return keys[key] && !prevKeys[key];
}

bool Input::getKeyUp(int key)
{
    return !keys[key] && prevKeys[key];
}

bool Input::getButton(int key)
{
    return mouseButtons[key];
}

bool Input::getButtonDown(int key)
{
    return mouseButtons[key] && !prevMouseButtons[key];
}

bool Input::getButtonUp(int key)
{
    return !mouseButtons[key] && prevMouseButtons[key];
}

glm::vec2 Input::getMousePosition()
{
    return mousePosition;
}

glm::vec2 Input::getMouseDelta()
{
    return mousePosition - prevMousePosition;
}

} // namespace core