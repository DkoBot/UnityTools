#include "engine.h"
#include "hooks.h"
#include "Conini.h"
#define DLL_QUERY_HMODULE		6

extern HINSTANCE g_hModule = nullptr;
void MainThread() {
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    Config::CreateConfig("Cofig1.ini");
    Engine::initialize();
    Hooks::initialize(kiero::RenderType::Auto);
    cout << "init successfully!" << endl;
    cout << "GitHub Link: https://github.com/DkoBot/UnityTools" << endl;

}
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_QUERY_HMODULE:
        if (lpReserved != NULL)
            *(HMODULE*)lpReserved = g_hModule;
    case DLL_PROCESS_ATTACH:
    {
        g_hModule = hModule; // 保存模块句柄
        HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MainThread, NULL, 0, NULL);
        if (hThread) {
            CloseHandle(hThread);
        }
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

