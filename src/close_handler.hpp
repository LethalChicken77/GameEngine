#ifndef CLOSE_HANDLER
#define CLOSE_HANDLER

#include <csignal>
#include <iostream>
#include "utils/console.hpp"
#if WIN32
#include "windows_attach_console.hpp"
#include <io.h>
#endif
// Only to be used by main

void OnExit()
{
    Console::saveLog();
    std::cout << "Exited successfully" << std::endl;
}

void OnSignal(int sig)
{
    #ifdef _WIN32
    switch(sig)
    {
        case SIGSEGV:
            _write(_fileno(stdout), "Segmentation fault\n", 19); // Because windows doesn't like to tell me things
            break;
        case SIGFPE:
            _write(_fileno(stdout), "Floating point error\n", 21);
            break;
        case SIGILL:
            _write(_fileno(stdout), "Illegal instruction\n", 20);
            break;
        case SIGABRT:
            _write(_fileno(stdout), "Aborted\n", 8);
            break;
        case SIGTERM:
            _write(_fileno(stdout), "Terminated\n", 11);
            break;
    }
    // ShutdownConsole();
    #endif
    // Save log file
    Console::saveLog();
}

void HandleSignals()
{
    atexit(OnExit);

    std::signal(SIGABRT, OnSignal);
    std::signal(SIGINT, OnSignal);
    std::signal(SIGTERM, OnSignal);
    std::signal(SIGSEGV, OnSignal);
    std::signal(SIGFPE, OnSignal);

}
#endif