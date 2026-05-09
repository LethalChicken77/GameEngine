#pragma once
#ifdef WIN32
#include <windows.h>

#include <iostream>
#include <cstdio>

void InitConsoleIO()
{
    FILE* fp;

    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    freopen_s(&fp, "CONIN$", "r", stdin);

    std::ios::sync_with_stdio(true);
}

BOOL WINAPI ConsoleHandler(DWORD signal)
{
    switch (signal)
    {
        case CTRL_C_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            Console::saveLog();
            ExitProcess(0);
            return TRUE;
    }
    return FALSE;
}

void SetupConsole()
{
    if (AttachConsole(ATTACH_PARENT_PROCESS))
    {
        InitConsoleIO();
        SetConsoleCtrlHandler(ConsoleHandler, TRUE);
    }
    // else
    // {
    //     AllocConsole();
    //     InitConsoleIO();
    // }
}

void ShutdownConsole()
{
    std::cout << std::endl;
    fflush(stdout);
    fflush(stderr);

    fclose(stdin);
    fclose(stdout);
    fclose(stderr);
}

#endif