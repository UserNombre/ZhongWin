#pragma once

#include "config.hpp"

namespace zw::hooks
{
    bool init();
    void shutdown();

    namespace platform
    {
        bool hook();
        void unhook();
        bool init();
        void shutdown();
        void begin_render();
    }

    namespace graphics
    {
        bool hook();
        void unhook();
        bool init();
        void shutdown();
        void begin_render();
        void end_render();
    }
}