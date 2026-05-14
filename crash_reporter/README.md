# crash_reporter

A small standalone executable that displays crash log output in an ImGui window when the engine or editor encounters a fatal error.

## What It Does

When Scion2D crashes, the `CrashLogger` (in `SCION_LOGGER`) writes a crash entry to a log file and then launches `SCION_CRASH_REPORTER` as a child process, passing the path to that log file as a command-line argument.

The crash reporter opens the log file, extracts the most recent crash entry (identified by the `[CRITICAL] Program crashed!` separator), and displays it in a scrollable ImGui window. The user can read the crash details and dismiss the window with the OK button.

## Why It Is Separate

The crash reporter is intentionally a separate executable with no dependency on SCION_CORE or any engine systems. This means it can launch and display the log even if the engine itself is in a broken state. It links only against SDL3, glad, and ImGui.

## How It Is Launched

The `CrashLogger` in `SCION_LOGGER` spawns the crash reporter process after writing the log. You do not launch it manually during normal use. The path to the crash reporter executable must be reachable from wherever the engine or editor binary is run.

## Command-Line Usage

```
SCION_CRASH_REPORTER.exe <path_to_crash_log>
```

If no path is provided the executable shows an SDL message box error and exits immediately.

## Log Format

The crash reporter looks for entries beginning with `[CRITICAL] Program crashed!` as a section delimiter. If the log contains multiple crash entries, only the most recent one is shown. The full log file is not displayed -- only the last entry -- to keep the output relevant to the crash that just occurred.

## Window Behavior

- Launched in release builds with the console window hidden (`SW_HIDE` on Windows).
- Launched in debug builds with the console window visible so you can see any diagnostic output alongside the ImGui window.
- On Windows, triggers `MessageBeep(MB_ICONERROR)` at launch so the user gets an audio alert.
- The ImGui window is fixed to fill the entire 640x480 display. It is not resizable or movable.
- Uses OpenGL 3.3 core (not 4.6) since it does not need any advanced rendering features.

## Dependencies

`SDL3`, `glad`, `ImGui` (no docking required)
