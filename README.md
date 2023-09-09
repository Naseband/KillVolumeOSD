# KillVolumeOSD

Kills Windows 10's annoying Volume and Brightness display until the next time explorer is completely restarted.

I only made this version as it is a lot simpler and I do not ever want to restore it. I also used a slightly different method to bring up the OSD (no key sends to the active window).

# Launch options

There are two available command line parameters:

"-debug" will enable logging to log.txt.

"-delay" will delay the launch by five seconds.

You can launch this program at startup either by adding a shortcut to your startup directory (Win + R, "shell:startup") or create a task in task scheduler for individual users (Win + R, "taskschd.msc").

When using a task, it's usually best to add a log on event trigger. There is a configurable delay aswell.

The audio files in the release are optional, you can remove them if you want this program to be silent.

# Building

The project was created with VS2019 and requires C++17 or C++20 to build.

# Credits

Credits to Marcus Venturi for finding the method used to detect the window.

https://github.com/UnlimitedStack/HideVolumeOSD

I suggest using that version if you want more flexibility, or the ability to restore it without restarting explorer.
