#pragma once
#include <windows.h>

namespace zw
{
    bool init(HMODULE hModule);
    void shutdown();

    void WINAPI main(HMODULE hModule);

    enum struct Error
    {
        NONE,
        LOG,
        HOOKS,
        UI,
        DICTIONARY
    };

    struct State
    {
        bool signal;
        bool run;
    };

    extern Error LastError;
    extern struct State State;
    extern CRITICAL_SECTION StateLock;
    extern CONDITION_VARIABLE StateCondition;
}