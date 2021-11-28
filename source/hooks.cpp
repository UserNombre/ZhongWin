#include <windows.h>

#include "config.hpp"
#include "hooks.hpp"

namespace zw::hooks
{
    //------------------------------
    //-- Public functions
    //------------------------------

    // PRE:     Nothing is hooked and program is not initialized
    // POST:    If      both [platform::hook] and [graphics::hook] are successful
    //          Then    both {platform} and {graphics} are hooked and [true] is returned
    //          Else    program state is left unchanged* and [false] is returned
    bool
    init()
    {
        if(!platform::hook())
        {
            return false;
        }
        if(!graphics::hook())
        {
            platform::unhook();
            return false;
        }

        return true;
    }

    // PRE:     Both {platform} and {graphics} are hooked and program is not running
    // POST:    Both {platform} and {graphics} are unhooked
    void
    shutdown()
    {
        graphics::unhook();
        platform::unhook();
    }
}