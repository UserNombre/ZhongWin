#pragma once
#include <string>

namespace zw::log
{
    bool init();
    void shutdown();

    void write(PCSTR string);
    void writef(PCSTR format, ...);
}