#include "../config.hpp"
#if defined(ZW_WIN32)

//------------------------------------------------------------
//-- Windows32
//------------------------------------------------------------
#include <windows.h>

#include "imgui/imgui.h"
#include "imgui/impl/imgui_impl_win32.h"

#include "../zhongwin.hpp"
#include "../ui.hpp"
#include "../log.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace zw::hooks::win32
{
    //------------------------------
    //-- Public variables
    //------------------------------
    HWND Window = NULL;

    namespace
    {
        //------------------------------
        //-- Private variables
        //------------------------------
        LRESULT (CALLBACK *WindowProc)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) = NULL;

        //------------------------------
        //-- Private functions
        //------------------------------
        BOOL CALLBACK
        enumGetProcessWindow(HWND hwnd, LPARAM lParam)
        {
            DWORD dwProcessId;
            ::GetWindowThreadProcessId(hwnd, &dwProcessId);
            if(dwProcessId == lParam)
            {
#ifdef _DEBUG
                WCHAR wszName[MAX_PATH];
                if(::GetClassName(hwnd, wszName, MAX_PATH) && wcscmp(wszName, L"ConsoleWindowClass") == 0)
                    return TRUE;
#endif
                Window = hwnd;
                return FALSE;
            }

            return TRUE;
        }

        LRESULT CALLBACK
        hkWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
        {
            if(zw::State.run)
            {
                //FIXME: prevent clicking through window
                zw::ui::input(msg, wParam, lParam);
                if(ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
                    return true;
            }

            return CallWindowProc((WNDPROC)WindowProc, hWnd, msg, wParam, lParam);
        }
    }

    //------------------------------
    //-- Public functions
    //------------------------------

    // PRE:     Nothing is hooked and program is not initialized
    // POST:    If      [Window] can be initialized and [SetWindowLongPtr] is successful
    //          Then    [Window]'s [WinProc] is hooked with [hkWindowProc] and [true] is returned
    //          Else    program state is left unchanged* and [false] is returned
    bool
    hook()
    {
        ::EnumWindows(enumGetProcessWindow, ::GetCurrentProcessId());
        if(!Window)
            return false;

        WindowProc = (WNDPROC)::SetWindowLongPtr(Window, GWLP_WNDPROC, (LONG_PTR)hkWindowProc);
        if(!WindowProc)
        {
            Window = NULL;
            return false;
        }

        return true;
    }

    // PRE:     Only {win32} is hooked and program is not initialized
    // POST:    [Window]'s [WinProc] is unhooked
    void
    unhook()
    {
        if(WindowProc)
            ::SetWindowLongPtr(Window, GWLP_WNDPROC, (LONG_PTR)WindowProc);

        WindowProc = NULL;
        Window = NULL;
    }

    bool
    init()
    {
        if(!ImGui_ImplWin32_Init(Window))
            return false;
        ImGui_ImplWin32_EnableDpiAwareness();

        return true;
    }

    void
    shutdown()
    {
        ImGui_ImplWin32_Shutdown();
    }

    void
    begin_render()
    {
        ImGui_ImplWin32_NewFrame();
    }
}

#endif