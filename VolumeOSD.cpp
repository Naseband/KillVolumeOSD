#include <Windows.h>
#include <WinUser.h>
#include <string>
#include <fstream>
#include <format>

#pragma comment(lib, "Winmm.lib") // for audio

// ------------------------------------------------------------ 

enum class eHideMethod
{
    DESTROY,    // Destroy the window. Never shows again unless explorer.exe is restarted.
    HIDE,       // Hides the window like in HideVolumeOSD, but may occasionally bug out (should to be run in daemon mode).
    NOINPUT     // Show the window as usual, but prevent any mouse or keyboard interaction.
};

enum class eHideResult
{
    FAIL,       // Plays failure sound and shows error.
    SUCCESS,    // Plays success sound.
    NOCHANGE    // Does nothing.
};

constexpr const wchar_t* GetHideMethodName(eHideMethod method)
{
    switch (method)
    {
    case eHideMethod::DESTROY:
        return L"Destroy";

    case eHideMethod::HIDE:
        return L"Hide";

    case eHideMethod::NOINPUT:
        return L"No Input";
    }

    return L"Unknown";
}

std::wofstream g_LogFile;

constexpr int NUM_TRIES = 10;

// ------------------------------------------------------------ 

int ShowError(const std::wstring& text)
{
    mciSendString(L"open \"notify-error.mp3\" type mpegvideo alias mp3", NULL, 0, NULL);
    mciSendString(L"play mp3 wait", NULL, 0, NULL);
    mciSendString(L"close mp3", NULL, 0, NULL);

    MessageBox(NULL, text.c_str(), L"KillVolumeOSD", 0);

    return 1;
}

void Log(const std::wstring& text)
{
    g_LogFile << text << std::endl;
}

eHideResult HideWindow(HWND window, eHideMethod method)
{
    switch (method)
    {
    case eHideMethod::DESTROY:
    {
        if (PostMessageW(window, WM_DESTROY, 0, 0))
            return eHideResult::SUCCESS;
        else
            return eHideResult::FAIL;
    }

    case eHideMethod::HIDE:
    {
        auto style{ GetWindowLongA(window, GWL_STYLE) };

        if (style & (WS_ICONIC | WS_DISABLED))
            return eHideResult::NOCHANGE;

        if (style == 0)
            return eHideResult::FAIL;

        // WS_ICONIC means the window is minimized when shown.
        // Also set WS_DISABLED so in the rare case it does pop up it's not clickable.
        SetWindowLongA(window, GWL_STYLE, style | (WS_ICONIC | WS_DISABLED));

        return eHideResult::SUCCESS;
    }

    case eHideMethod::NOINPUT:
    {
        auto style{ GetWindowLongA(window, GWL_STYLE) };

        if (style & WS_DISABLED)
            return eHideResult::NOCHANGE;

        if (style == 0)
            return eHideResult::FAIL;

        // Setting WS_DISABLED doesn't hide it, but prevents any input from being processed.
        SetWindowLongA(window, GWL_STYLE, style | WS_DISABLED);
        
        return eHideResult::SUCCESS;
    }
    }

    // May work if setting the initial window pos (but it doesn't as-is):
    // SetWindowPos(window, NULL, -10000, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOSENDCHANGING);

    return eHideResult::NOCHANGE;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    hPrevInstance;

    std::wstring params{ lpCmdLine };

    bool is_debug{ false };
    bool is_daemon{ false };
    bool use_delay{ false };
    eHideMethod method{ eHideMethod::DESTROY };

    // Parse command line args

#if defined _DEBUG

    is_debug = true;

#else

    if (params.find(L"-debug") != params.npos)
        is_debug = true;

#endif

    if (is_debug)
        g_LogFile.open(L"log.txt", std::ios::out);

    if (params.find(L"-daemon") != params.npos)
        is_daemon = true;

    if (params.find(L"-delay") != params.npos)
        use_delay = true;

    if (params.find(L"-destroy") != params.npos)
        method = eHideMethod::DESTROY;
    else if (params.find(L"-hide") != params.npos)
        method = eHideMethod::HIDE;
    else if (params.find(L"-noinput") != params.npos)
        method = eHideMethod::NOINPUT;

    Log(L"-- Killing VolumeOSD");
    Log(std::format(L"- Debug: {}", is_debug ? L"Yes" : L"No"));
    Log(std::format(L"- Daemon: {}", is_daemon ? L"Yes" : L"No"));
    Log(std::format(L"- Delay: {}", use_delay ? L"Yes" : L"No"));
    Log(std::format(L"- Method: {}", GetHideMethodName(method)));

    // Get the explorer window handle

    HWND window_explorer{ NULL };

    do
    {
        Sleep(200);

        window_explorer = GetShellWindow();
    }
    while (window_explorer == NULL);

    Log(std::format(L"GetShellWindow HWND = {}", (void*)window_explorer));

    // Delay

    if (use_delay)
        Sleep(5000);

    // Show the VolumeOSD (only once)

    Log(L"-- Sending Mute Key messages ...");

    SendMessageW(window_explorer, WM_APPCOMMAND, 0, APPCOMMAND_VOLUME_MUTE << 16);
    SendMessageW(window_explorer, WM_APPCOMMAND, 0, APPCOMMAND_VOLUME_MUTE << 16);

    // Run main loop - only runs once if is_daemon is false

    HWND window_target{ NULL };

    int tries{ 0 };

    while (is_daemon || tries++ < NUM_TRIES)
    {
        if(!is_daemon)
            Log(std::format(L"Try: {}", tries));

        do
        {
            HWND window_parent{ NULL };

            window_target = NULL;

            while ((window_parent = FindWindowEx(NULL, window_parent, L"NativeHWNDHost", L"")) != NULL)
            {
                Log(std::format(L"Parent candidate window HWND = {}", (void*)window_parent));

                if (FindWindowExW(window_parent, NULL, L"DIRECTUIHWND", L"") != NULL)
                {
                    if (window_target == NULL)
                    {
                        Log(std::format(L"Target HWND = {}", (void*)window_parent));

                        window_target = window_parent;
                    }
                }
            }

            if (window_target == NULL)
                Sleep(500);
        }
        while (window_target == NULL);

        // Only do something if the window is actually visible

        if (IsWindowVisible(window_target))
        {
            switch (HideWindow(window_target, method))
            {
            case eHideResult::SUCCESS:
                Log(L"VolumeOSD Window hidden successfully");

                mciSendString(L"open \"notify-success.mp3\" type mpegvideo alias mp3", NULL, 0, NULL);
                mciSendString(L"play mp3 wait", NULL, 0, NULL);
                mciSendString(L"close mp3", NULL, 0, NULL);
                break;

            case eHideResult::FAIL:
                Log(L"Failed to hide VolumeOSD window");

                ShowError(L"Failed to hide VolumeOSD window.");
                break;
            }

            if (!is_daemon)
                break;
        }

        if (is_daemon)
        {
            Log(L"-- Waiting for next check");
            Sleep(2500);
        }
        else
        {
            Sleep(60 + tries * 40);
        }
    }

    return 0;
}

// ------------------------------------------------------------ EOF