# Winamp EQ Hotkey

**Version:** 1.4.2  

## Description
Global hotkey plugin to toggle the Winamp equalizer (EQ) on and off.  
Supports both installed and portable versions of Winamp and saves the hotkey configuration automatically.

## Features
- Set a global hotkey (default: CTRL+SHIFT+E) to toggle on/off the EQ.
- Detects Winamp portable installation and *paths.ini*.
- Saves hotkey settings in *%APPDATA%\Winamp\Plugins\gen_eqhotkey.ini* or the path specified in paths.ini.

## Installation
1. **Download a precompiled release** or compile the plugin as a DLL using any Win32-compatible C++
2. Copy *gen_eqhotkey.dll* to the Plugins folder of Winamp.
3. Open Winamp and enable the plugin in General Purpose Plugins.



## Build Example
```bash
g++ -shared -o gen_eqhotkey.dll src/main.cpp -Iinclude -lcomctl32 -luser32 -lshell32
```

## Compiling with CMake
1. Clone the repository.
2. Open a terminal in the project folder.
3. Run `cmake -B build` to generate the build files.
4. Run `cmake --build build` to compile the DLL.

## Screenshot:

<img width="288" height="163" alt="immagine" src="https://github.com/user-attachments/assets/88eef343-18a8-41ce-8b4d-c778cc029326" />
