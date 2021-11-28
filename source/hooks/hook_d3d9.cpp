#include "../config.hpp"
#if defined(ZW_D3D9)

//------------------------------------------------------------
//-- DirectX9
//------------------------------------------------------------
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

#include "imgui/impl/imgui_impl_dx9.h"
#include "detours/detours.h"
#pragma comment(lib, "detours.lib")

#include "../zhongwin.hpp"
#include "../ui.hpp"
#include "../log.hpp"
#include "hook_win32.hpp"

namespace zw::hooks::d3d9
{
    namespace
    {
        //------------------------------
        //-- Private variables
        //------------------------------
        IDirect3DDevice9 *Device = NULL;

        HRESULT(__stdcall *EndScene)(IDirect3DDevice9 *pDevice) = NULL;
        constexpr auto EndSceneIdx = 42;

        //------------------------------
        //-- Private functions
        //------------------------------
        HRESULT __stdcall
        hkEndScene(IDirect3DDevice9 *pDevice)
        {
            if(!Device)
            {
                Device = pDevice;

                EnterCriticalSection(&zw::StateLock);
                zw::State.signal = true;
                LeaveCriticalSection(&zw::StateLock);
                WakeConditionVariable(&zw::StateCondition);
            }
            else if(zw::State.run)
            {
                zw::ui::render();
            }

            return EndScene(pDevice);
        }
    }

    //------------------------------
    //-- Public functions
    //------------------------------

    // PRE:     Only {win32} is hooked and program is not initialized
    // POST:    If      [d3d9] and [d3d9Device] can be initialized and [DetourAttach] is successful
    //          Then    [d3d9Device]'s [EndScene] is hooked and [true] is returned
    //          Else    program state is left unchanged* and [false] is returned
    bool
    hook()
    {
        IDirect3D9 *d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
        if(!d3d9)
            return false;

        IDirect3DDevice9 *d3d9Device = NULL;
        D3DPRESENT_PARAMETERS d3d9Parameters = {};
        d3d9Parameters.Windowed = true;
        d3d9Parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
        d3d9Parameters.hDeviceWindow = win32::Window;

        HRESULT result = d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3d9Parameters.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3d9Parameters, &d3d9Device);
        if(FAILED(result))
        {
            d3d9Parameters.Windowed = false;
            result = d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3d9Parameters.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3d9Parameters, &d3d9Device);
            if(FAILED(result))
            {
                d3d9->Release();
                return false;
            }
        }

        (PVOID &)EndScene = (*(PVOID **)d3d9Device)[EndSceneIdx];

        d3d9Device->Release();
        d3d9->Release();

        DetourTransactionBegin();
        DetourUpdateThread(::GetCurrentThread());
        LONG error = DetourAttach(&(PVOID &)EndScene, hkEndScene);
        DetourTransactionCommit();

        return NO_ERROR == error;
    }

    // PRE:     Both {win32} and {d3d9} are hooked and program is not initialized
    // POST:    [Device]'s [EndScene] is unhooked
    void
    unhook()
    {
        DetourTransactionBegin();
        DetourUpdateThread(::GetCurrentThread());
        LONG error = DetourDetach(&(PVOID &)EndScene, hkEndScene);
        DetourTransactionCommit();
        assert(!error);

        EndScene = NULL;
        Device = NULL;
    }

    bool
    init()
    {
        if(!ImGui_ImplDX9_Init(Device))
            return false;
        return true;
    }

    void
    shutdown()
    {
        ImGui_ImplDX9_Shutdown();
    }

    void
    begin_render()
    {
        ImGui_ImplDX9_NewFrame();
    }

    void
    end_render()
    {
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    }
}

#endif