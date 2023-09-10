#include <Windows.h>
#include <WinUser.h>
#include <string>
#include <fstream>
#include <format>

#pragma comment(lib, "Winmm.lib") // for audio

using namespace std;

bool g_bDebug = false;

int ShowError(wstring Text)
{
    mciSendString(L"open \"notify-error.mp3\" type mpegvideo alias mp3", NULL, 0, NULL);
    mciSendString(L"play mp3 wait", NULL, 0, NULL);
    mciSendString(L"close mp3", NULL, 0, NULL);

    MessageBox(NULL, Text.c_str(), L"KillVolumeOSD", 0);

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

    bool bDaemon = false;
    bool bDelay = false;

#if defined _DEBUG

    g_bDebug = true;

#else

    if (Params.find(L"-debug") != Params.npos)
        g_bDebug = true;

#endif

    if (Params.find(L"-daemon") != Params.npos)
        bDaemon = true;

    if (Params.find(L"-delay") != Params.npos)
        bDelay = true;

    Log(L"--- Killing VolumeOSD");
    Log(format(L"-- Debug: {}", g_bDebug ? L"Yes" : L"No"));
    Log(format(L"-- Delay: {}", bDelay ? L"Yes" : L"No"));
    Log(format(L"-- Daemon: {}", bDaemon ? L"Yes" : L"No"));

    HWND hWndExplorer;

    do
    {
        Sleep(200);

        hWndExplorer = GetShellWindow();
    }
    while (hWndExplorer == NULL);

    Log(format(L"GetShellWindow HWND = {}", (void*)hWndExplorer));

    if (bDelay)
        Sleep(5000);

    PostMessage(hWndExplorer, WM_APPCOMMAND, 0, APPCOMMAND_VOLUME_MUTE << 16);
    PostMessage(hWndExplorer, WM_APPCOMMAND, 0, APPCOMMAND_VOLUME_MUTE << 16);

    HWND hWndTarget;

    do
    {
        do
        {
            Sleep(700);

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
                        Log(format(L"Target HWND = {}", (void*)hWndParent));

                        hWndTarget = hWndParent;
                    }
                }
            }
        }
        while (hWndTarget == NULL);

        if (hWndTarget != NULL)
        {
            Log(format(L"Target window belongs to PID {}", GetWindowThreadProcessId(hWndTarget, 0)));

            Log(format(L"Window is visible: {}", IsWindowVisible(hWndTarget) ? L"Yes" : L"No"));

            if (IsWindowVisible(hWndTarget))
            {
                if (HideWindow(hWndTarget))
                {
                    Log(L"VolumeOSD Window hidden successfully");

                    mciSendString(L"open \"notify-success.mp3\" type mpegvideo alias mp3", NULL, 0, NULL);
                    mciSendString(L"play mp3 wait", NULL, 0, NULL);
                    mciSendString(L"close mp3", NULL, 0, NULL);
                }
                else
                {
                    Log(L"Failed to hide VolumeOSD window");

                    ShowError(L"Failed to hide VolumeOSD window.");
                }
            }
        }

        if (bDaemon)
            Log(L"-- Waiting for next check");

        Sleep(3000);
    }
    while (bDaemon);

    return 0;
}