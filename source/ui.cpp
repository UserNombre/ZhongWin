#include <string>

#include "utfcpp/utf8.h"
#include "imgui/imgui.h"
#include "imgui/imgui_stdlib.h"

#include "ui.hpp"
#include "zhongwin.hpp"
#include "hooks.hpp"
#include "dictionary.hpp"
#include "log.hpp"

namespace zw::ui
{
    namespace
    {
        //------------------------------
        //-- Private variables
        //------------------------------
        IDictionary* Dictionary = NULL;
        int SelectionPos = -1;
        int SelectionLen = -1;
        std::string SelectionText = u8"你好世界!";
        std::string DefinitionText = u8"";

        //------------------------------
        //-- Private functions
        //------------------------------
        void
        update_definition(std::vector<std::string> entries)
        {
            std::string text;
            for(std::string entry : entries)
            {
                text = text + entry + "\n\n";
            }
            DefinitionText = text;
        }

        int
        cbSelectionText(ImGuiInputTextCallbackData* data)
        {
            if(data->BufTextLen != SelectionLen)
            {
                update_selection(data->Buf);
                SelectionLen = data->BufTextLen;
            }
            if(data->CursorPos != SelectionPos)
            {
                std::vector<std::string> entries = Dictionary->search_string(SelectionText, data->CursorPos);
                update_definition(entries);
                SelectionPos = data->CursorPos;
            }

            return 0;
        }

        void
        render_selection()
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
            ImGui::BeginChild("SelectionWindow", ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetWindowContentRegionHeight() / 2), true);

            ImGuiInputTextFlags textFlags = ImGuiInputTextFlags_CallbackAlways;
            ImGui::InputTextMultiline("SelectionText", &SelectionText, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), textFlags, cbSelectionText);
            
            ImGui::EndChild();
            ImGui::PopStyleVar();
        }

        void
        render_definition()
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
            ImGui::BeginChild("DefinitionWindow", ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetWindowContentRegionHeight() / 2), true);
            ImGui::TextWrapped(DefinitionText.c_str());
            ImGui::EndChild();
            ImGui::PopStyleVar();
        }
    }

    //------------------------------
    //-- Public functions
    //------------------------------
    ZHONGWIN_API void
    update_selection(const char* text)
    {
        SelectionText = std::string(text);
    }

    // PRE:     Both {platform} and {graphics} are hooked and program is not initialized
    // POST:    If      [zw::hooks::platform::init] and [zw::hooks::graphics::init] are successful
    //          Then    program is initialized and [true] is returned
    //          Else    program state is left unchanged* and [false] is returned
    bool
    init(IDictionary* dictionary)
    {
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

        Dictionary = dictionary;
        ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\SimKai.ttf", 20.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
        IM_ASSERT(font != NULL);

        if(!zw::hooks::platform::init())
        {
            log::write("hooks::platform::init failed");
            ImGui::DestroyContext();
            return false;
        }
        
        if(!zw::hooks::graphics::init())
        {
            log::write("hooks::graphics::init failed");
            zw::hooks::platform::shutdown();
            ImGui::DestroyContext();
            return false;
        }

        return true;
    }

    // PRE:     Both {platform} and {graphics} are hooked and program is not initialized
    // POST:    Program is uninitialized
    void
    shutdown()
    {
        zw::hooks::graphics::shutdown();
        zw::hooks::platform::shutdown();
        ImGui::DestroyContext();
    }

    void
    render()
    {
        zw::hooks::platform::begin_render();
        zw::hooks::graphics::begin_render();

        ImGui::NewFrame();

        ImGui::Begin("ZhongWin");
        render_selection();
        ImGui::Separator();
        render_definition(); 
        ImGui::End();

        ImGui::EndFrame();
        ImGui::Render();

        zw::hooks::graphics::end_render();
    }

    void
    input(UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch(msg)
        {
        case WM_KEYDOWN:
            if(VK_DELETE == wParam)
            {
                EnterCriticalSection(&zw::StateLock);
                zw::State.run = false;
                LeaveCriticalSection(&zw::StateLock);
                WakeConditionVariable(&zw::StateCondition);
            }
            break;
        }
    }
}