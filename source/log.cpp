#include <string>
#include <iomanip>
#include <fstream>
#include <chrono>
#include <cstdio>
#include <cstdarg>
#include <windows.h>

#ifdef _DEBUG
#include "debugwin/debugwin.hpp"
#pragma comment(lib, "debugwin.lib")
#endif

#include "log.hpp"

namespace zw::log
{
    namespace
    {
        //------------------------------
        //-- Private variables
        //------------------------------
        std::ofstream logFile;
        std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    }

    //------------------------------
    //-- Public functions
    //------------------------------
    bool
    init()
    {
#ifdef _DEBUG
        if(!::GetModuleHandle(L"debugwin.dll"))
            return false;
#endif
        logFile.open("zhongwin.log", std::ios_base::binary | std::ios_base::app);
        if(!logFile.is_open())
            return false;

        startTime = std::chrono::high_resolution_clock::now();

        return true;
    }

    void
    shutdown()
    {
        logFile.close();
    }

    void
    write(PCSTR string)
    {
#ifdef _DEBUG
        dw_log_string(string);
#endif

        auto currentTime = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(currentTime - startTime).count();
        logFile << std::fixed << std::setprecision(2)
                << '[' << elapsed/1000 << "] " << string << '\n';
    }

    void
    writef(PCSTR format, ...)
    {
        CHAR buffer[256];

        va_list va;
        va_start(va, format);
        _vsnprintf_s(buffer, _TRUNCATE, format, va);
        va_end(va);

        log::write(buffer);
    }
}