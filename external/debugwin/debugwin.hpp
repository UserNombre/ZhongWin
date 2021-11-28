#pragma once
#include <string>

#ifdef DEBUGWIN_EXPORTS
#define DEBUGWIN_API __declspec(dllexport)
#else
#define DEBUGWIN_API __declspec(dllimport)
#endif

DEBUGWIN_API void dw_log_string(PCSTR message);
DEBUGWIN_API void dw_log(PCSTR format, ...);
DEBUGWIN_API void dw_vlog(PCSTR format, va_list *va);

DEBUGWIN_API void dw_log_string(PCWSTR message);
DEBUGWIN_API void dw_log(PCWSTR format, ...);
DEBUGWIN_API void dw_vlog(PCWSTR format, va_list *va);