#include "CryCommon/CrySystem/ISystem.h"
#include "Library/WinAPI.h"

#include "SC_QuitHook.h"
#include "..\SC_Server.h"

struct CSystemHook
{
    void Quit();
};

using SC_QuitFunc = decltype(&CSystemHook::Quit);
static SC_QuitFunc g_SC_originalQuit = nullptr;

void CSystemHook::Quit()
{
    CryLogAlways("%s", __FUNCTION__);
    gSC_Server->Quit();

   (this->*g_SC_originalQuit)();
}

void SC_QuitHook::Init()
{
    void** pSystemVTable = *reinterpret_cast<void***>(gEnv->pSystem);

    g_SC_originalQuit = reinterpret_cast<SC_QuitFunc&>(pSystemVTable[14]);

    // vtable hook
    SC_QuitFunc newQuit = &CSystemHook::Quit;
    WinAPI::FillMem(&pSystemVTable[14], &reinterpret_cast<void*&>(newQuit), sizeof(void*));
}