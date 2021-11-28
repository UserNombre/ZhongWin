#include "../config.hpp"
#if defined(ZW_D3D11)

//------------------------------------------------------------
//-- DirectX11
//------------------------------------------------------------
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#include "imgui/impl/imgui_impl_dx11.h"
#include "detours/detours.h"
#pragma comment(lib, "detours.lib")

#include "../zhongwin.hpp"
#include "../ui.hpp"
#include "../log.hpp"
#include "hook_win32.hpp"

namespace zw::hooks::d3d11
{
    namespace
    {
        //------------------------------
        //-- Private variables
        //------------------------------
        ID3D11Device *Device = NULL;
        ID3D11DeviceContext *DeviceContext = NULL;
        ID3D11RenderTargetView *RenderTargetView = NULL;
        IDXGISwapChain *SwapChain = NULL;

        HRESULT(__stdcall *Present)(IDXGISwapChain *pSwapChain, UINT SyncInterval, UINT Flags) = NULL;
        constexpr auto PresentIdx = 8;

        //------------------------------
        //-- Private functions
        //------------------------------
        HRESULT __stdcall
        hkPresent(IDXGISwapChain *pSwapChain, UINT SyncInterval, UINT Flags)
        {
            if(!Device)
            {
                SwapChain = pSwapChain;
                HRESULT result = pSwapChain->GetDevice(__uuidof(ID3D11Device), (PVOID *)&Device);
                assert(SUCCEEDED(result));

                Device->GetImmediateContext(&DeviceContext);
                assert(NULL != DeviceContext);

                ID3D11Texture2D *pBackBuffer;
                result = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&pBackBuffer);
                assert(SUCCEEDED(result));
                result = Device->CreateRenderTargetView(pBackBuffer, NULL, &RenderTargetView);
                assert(SUCCEEDED(result));
                pBackBuffer->Release();

                EnterCriticalSection(&zw::StateLock);
                zw::State.signal = true;
                LeaveCriticalSection(&zw::StateLock);
                WakeConditionVariable(&zw::StateCondition);
            }
            else if(zw::State.run)
            {
                zw::ui::render();
            }

            return Present(pSwapChain, SyncInterval, Flags);
        }
    }

    //------------------------------
    //-- Public functions
    //------------------------------

    // PRE:     Only {win32} is hooked and program is not initialized
    // POST:    If      [d3d11] and [d3d11Device] can be initialized and [DetourAttach] is successful
    //          Then    [d3d11Device]'s [EndScene] is hooked and [true] is returned
    //          Else    program state is left unchanged* and [false] is returned
    bool
    hook()
    {
        ID3D11Device *d3d11Device = NULL;
        IDXGISwapChain *d3d11SwapChain = NULL;
        DXGI_SWAP_CHAIN_DESC dxgiDescriptor = {};
        dxgiDescriptor.BufferCount = 1;
        dxgiDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        dxgiDescriptor.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        dxgiDescriptor.OutputWindow = win32::Window;
        dxgiDescriptor.Windowed = true;
        dxgiDescriptor.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        dxgiDescriptor.SampleDesc.Count = 1;

        HRESULT result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0, D3D11_SDK_VERSION, &dxgiDescriptor, &d3d11SwapChain, &d3d11Device, NULL, NULL);
        if(FAILED(result))
        {
            dxgiDescriptor.Windowed = false;
            result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0, D3D11_SDK_VERSION, &dxgiDescriptor, &d3d11SwapChain, &d3d11Device, NULL, NULL);
            if(FAILED(result))
                return false;
        }

        (PVOID &)Present = (*(PVOID **)d3d11SwapChain)[PresentIdx];

        d3d11Device->Release();
        d3d11SwapChain->Release();

        DetourTransactionBegin();
        DetourUpdateThread(::GetCurrentThread());
        LONG error = DetourAttach(&(PVOID &)Present, hkPresent);
        DetourTransactionCommit();

        return NO_ERROR == error;
    }

    // PRE:     Both {win32} and {d3d11} are hooked and program is not initialized
    // POST:    [Device]'s [EndScene] is unhooked
    void
    unhook()
    {
        DetourTransactionBegin();
        DetourUpdateThread(::GetCurrentThread());
        LONG error = DetourDetach(&(PVOID &)Present, hkPresent);
        DetourTransactionCommit();
        assert(!error);

        Present = NULL;
        Device = NULL;
    }

    bool
    init()
    {
        if(!ImGui_ImplDX11_Init(Device, DeviceContext))
            return false;
        return true;
    }

    void
    shutdown()
    {
        ImGui_ImplDX11_Shutdown();
    }

    void
    begin_render()
    {
        ImGui_ImplDX11_NewFrame();
    }

    void
    end_render()
    {
        DeviceContext->OMSetRenderTargets(1, &RenderTargetView, NULL);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        Present(SwapChain, 1, 0);
    }
}

#endif