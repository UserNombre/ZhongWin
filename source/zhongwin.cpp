#include <string>
#include <windows.h>

#include "zhongwin.hpp"
#include "log.hpp"
#include "hooks.hpp"
#include "dictionary.hpp"
#include "ui.hpp"

namespace zw
{
    Error LastError = Error::NONE;
    struct State State{};
    CRITICAL_SECTION StateLock;
    CONDITION_VARIABLE StateCondition;

    namespace
    {
        //------------------------------
        //-- Private functions
        //------------------------------
        void
        wait_for_signal()
        {
            ::EnterCriticalSection(&StateLock);
            do
            {
                ::SleepConditionVariableCS(&StateCondition, &StateLock, INFINITE);
            } while(!State.signal);
            State.signal = false;
            ::LeaveCriticalSection(&StateLock);
        }

        void
        run()
        {
            State.run = true;
            ::EnterCriticalSection(&StateLock);
            do
            {
                ::SleepConditionVariableCS(&StateCondition, &StateLock, INFINITE);
            } while(State.run);
            ::LeaveCriticalSection(&StateLock);
        }
    }

    void WINAPI
    main(HMODULE hModule)
    {
        if(!log::init())
        {
            LastError = Error::LOG;
            goto exit;
        }
        IDictionary *dictionary;
        if(!hooks::init())
        {
            LastError = Error::HOOKS;
            log::write("hooks::init failed");
            goto exit_log;
        }

        zw::wait_for_signal();

        dictionary = new CEDICTDictionary(".\\data\\");
        if(!dictionary->isLoaded())
        {
            LastError = Error::DICTIONARY;
            log::write("dictionary failed");
            goto exit_hooks;
        }

        if(!ui::init(dictionary))
        {
            LastError = Error::UI;
            log::write("ui::init failed");
            goto exit_dict;
        }

        log::write("starting program");
        zw::run();
        log::write("stopping program");

    exit_ui:
        ui::shutdown();
    exit_dict:
        delete dictionary;
    exit_hooks:
        hooks::shutdown();
    exit_log:
        log::shutdown();
    exit:
        ::FreeLibraryAndExitThread(hModule, (DWORD)LastError);
    }

    bool
    init(HMODULE hModule)
    {
        ::DisableThreadLibraryCalls(hModule);
        ::InitializeCriticalSection(&StateLock);
        ::InitializeConditionVariable(&StateCondition);

        return true;
    }

    void
    shutdown()
    {
        ::DeleteCriticalSection(&StateLock);
    }
}