#pragma once
#include <string>
#include <windows.h>

#include "dictionary.hpp"

#ifdef ZHONGWIN_EXPORTS
#define ZHONGWIN_API __declspec(dllexport)
#else
#define ZHONGWIN_API __declspec(dllimport)
#endif

namespace zw::ui
{
    bool init(IDictionary* dictionary);
    void shutdown();

    void render();
    void input(UINT msg, WPARAM wParam, LPARAM lParam);

    ZHONGWIN_API void update_selection(const char* text);
}