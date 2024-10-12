# KillVolumeOSD

Kills Windows 10's annoying Volume and Brightness display until the next time explorer is completely restarted.

I only made this version as it is a lot simpler and I do not ever want to restore it. I also used a slightly different method to bring up the OSD (no key sends to the active window).

# Command Line arguments

|&nbsp;&nbsp;&nbsp;**Option**&nbsp;&nbsp;&nbsp;| **Effect** |
|----------------|---------------|
| `-debug`     | Logs debug messages to log.txt |
| `-delay`     | Uses an initial delay of 5 seconds (useful on Windows startup) |
| `-daemon`    | Keeps VolumeOSD running in the background. This prevents the OSD from showing again after explorer is restarted. Note that the OSD is only shown once on startup. If the OSD returns, a user-interaction is required for it to show again. |

# Hide Methods

The following hide methods are also set via command line arguments.
Only one of them can be used at a time.

|&nbsp;&nbsp;&nbsp;**Method**&nbsp;&nbsp;&nbsp;| **Effect**|
|----------------|------------|
| `-destroy` | (default) Destroys the window completely. Side effect: App Commands (like multimedia inputs) do not propagate (ie. Browser media playback). |
| `-hide` | Hides the window like in HideVolumeOSD and disables its keyboard and mouse input (just in case it does return). |
| `-noinput` | The window will show as usual, but keyboard and mouse input will be disabled. No accidental volume increase! |

# Launching

You can launch this program at startup either by adding a shortcut to your startup directory (Win + R, "shell:startup") or creating a task in task scheduler for individual users (Win + R, "taskschd.msc").

When using a task, it's usually best to add a log on event trigger. There is a configurable delay aswell.

The audio files in the release are optional, you can leave them away if you want this program to be silent.

# Known issues

When the OSD is hidden with the default method, media keys will not be propagated through the Windows 10 multimedia interface.

This is not a problem for most media players like Spotify, Winamp or VLC, however some applications rely on that interface exclusively (such as Firefox).

If you want to keep that interface working, use -hide or -noinput together with the -daemon option.

# Building

The project was created with VS2019 and requires C++17 or C++20 to build.

# Thanks

Thanks to Marcus Venturi for finding the method used to detect the window.

https://github.com/UnlimitedStack/HideVolumeOSD

I suggest using that version if you want more flexibility, or the ability to restore it without restarting explorer.
