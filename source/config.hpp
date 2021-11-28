#pragma once

#define ZW_WIN32
#define ZW_D3D9

#if defined(ZW_WIN32)
#define platform win32
#else
#error Invalid platform
#endif

#if defined(ZW_D3D9)
#define graphics d3d9
#elif defined(ZW_D3D11)
#define graphics d3d11
#else
#error Invalid graphics library
#endif