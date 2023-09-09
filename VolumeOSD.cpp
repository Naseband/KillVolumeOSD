#include <Windows.h>
#include <WinUser.h>
#include <string>
#include <fstream>
#include <format>

#pragma comment(lib, "Winmm.lib") // for audio

using namespace std;

bool g_bDebug = false;

int ShowError(const char* szText)
{
    mciSendString(L"open \"notify-error.mp3\" type mpegvideo alias mp3", NULL, 0, NULL);
    mciSendString(L"play mp3 wait", NULL, 0, NULL);
    mciSendString(L"close mp3", NULL, 0, NULL);

    MessageBoxA(NULL, szText, "KillVolumeOSD", 0);

    return 1;
}

bool HideWindow(HWND hWnd)
{
    return PostMessage(hWnd, WM_DESTROY, 0, 0);
}

void Log(wstring Text)
{
    if (!g_bDebug)
        return;

    wofstream File(L"log.txt", ios::app);

    if (File.is_open())
        File << Text << endl;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    hPrevInstance;

    wstring Params{ lpCmdLine };

#if defined _DEBUG

    g_bDebug = true;

#else

    if (Params.find(L"-debug") != Params.npos)
        g_bDebug = true;
#endif

    if (Params.find(L"-delay") != Params.npos)
        Sleep(5000);

    Log(L"--- Killing VolumeOSD");

    HWND hWndExplorer = GetShellWindow();

    Log(format(L"GetShellWindow HWND = {}", (void*)hWndExplorer));

    if (hWndExplorer == NULL)
    {
        return ShowError("Failed to find Shell proxy window.");
    }

    PostMessage(hWndExplorer, WM_APPCOMMAND, 0, APPCOMMAND_VOLUME_MUTE << 16);
    PostMessage(hWndExplorer, WM_APPCOMMAND, 0, APPCOMMAND_VOLUME_MUTE << 16);

    HWND hWndTarget;

    ULONGLONG ullTimeoutStartTick = GetTickCount64();

    do
    {
        Sleep(200);

        HWND hWndParent = NULL;

        hWndTarget = NULL;
        
        while ((hWndParent = FindWindowEx(NULL, hWndParent, L"NativeHWNDHost", L"")) != NULL)
        {
            Log(format(L"Parent candidate window HWND = {}", (void*)hWndParent));

            if (FindWindowExW(hWndParent, NULL, L"DIRECTUIHWND", L"") != NULL)
            {
                Log(L"Found child window");

                if (hWndTarget == NULL)
                {
                    Log(format(L"Child window HWND = {}", (void*)hWndParent));

                    hWndTarget = hWndParent;
                }
            }
        }
    }
    while (hWndTarget == NULL && GetTickCount64() - ullTimeoutStartTick < 10000);

    if (hWndTarget == NULL)
    {
        return ShowError("Failed to find VolumeOSD after timeout.");
    }

    Log(format(L"Target window belongs to PID {}", GetWindowThreadProcessId(hWndTarget, 0)));

    if (!HideWindow(hWndTarget))
    {
        return ShowError("Failed to hide VolumeOSD window.");
    }

    mciSendString(L"open \"notify-success.mp3\" type mpegvideo alias mp3", NULL, 0, NULL);
    mciSendString(L"play mp3 wait", NULL, 0, NULL);
    mciSendString(L"close mp3", NULL, 0, NULL);

    return 0;
}