#include <iostream>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <memory>

#include "modules.hpp"
#include "utils/console.hpp"
#include "engine.hpp"
#include "utils/debug.hpp"
#include "core/input.hpp"

#include "close_handler.hpp"
#if WIN32
#include <windows.h>
#include <windows_attach_console.hpp>
#endif


using namespace core;
using namespace graphics;

#if WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd) 
{
    // SetupConsole();
    
    HandleSignals();
    // graphicsModule.init(APPLICATION_NAME, ENGINE_NAME);
    
    if(!graphicsModule.IsOpen())
    {
        Console::error("Failed to initialize graphics");
        return -1;
    }

    Engine engine{};

    engine.run();
    
    // graphicsModule.WaitForDevice();
    // graphicsModule.cleanup();

    return 0;
}
#else
int main() 
{
    HandleSignals();
    // graphicsModule.init(APPLICATION_NAME, ENGINE_NAME);
    
    if(!graphicsModule.IsOpen())
    {
        Console::error("Failed to initialize graphics");
        return -1;
    }

    Engine engine{};

    engine.run();
    
    // graphicsModule.WaitForDevice();
    // graphicsModule.cleanup();

    return 0;
}
#endif