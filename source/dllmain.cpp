#include <windows.h>

#include "zhongwin.hpp"

BOOL APIENTRY
DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if(DLL_PROCESS_ATTACH == ul_reason_for_call)
    {
        if(!zw::init(hModule))
            return FALSE;

        HANDLE hThread = ::CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)zw::main, hModule, NULL, NULL);
        if(!hThread)
            return FALSE;
        ::CloseHandle(hThread);
    }

    return TRUE;
}