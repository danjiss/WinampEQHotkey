#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#include <windows.h>
#include <commctrl.h>
#include <string>
#include "winamp/winamp_plugin.h"
#include <cstdio>
#include <cstring>
#include <shlobj.h>

HINSTANCE hInstance;
HWND hwndWinamp = NULL;
HWND hwndHidden = NULL;
const UINT HOTKEY_ID = 1;

UINT currentHotkeyModifiers = MOD_CONTROL | MOD_SHIFT;
UINT currentHotkeyKey = 0x45;

char iniPath[MAX_PATH] = {0};

BOOL WritePrivateProfileInt(LPCSTR lpAppName, LPCSTR lpKeyName, int nValue, LPCSTR lpFileName) {
    char buf[32];
    sprintf(buf, "%d", nValue);
    return WritePrivateProfileString(lpAppName, lpKeyName, buf, lpFileName);
}

void InitIniPath() {
    char winampPath[MAX_PATH] = {0};
    char pathsIni[MAX_PATH] = {0};
    char inidirValue[MAX_PATH] = {0};

    if (GetModuleFileName(NULL, winampPath, MAX_PATH) > 0) {
        char* lastSlash = strrchr(winampPath, '\\');
        if (lastSlash) *lastSlash = 0;
        sprintf(pathsIni, "%s\\paths.ini", winampPath);
    }

    bool validInidir = false;

    if (GetFileAttributesA(pathsIni) != INVALID_FILE_ATTRIBUTES) {
        if (GetPrivateProfileStringA("Winamp", "inidir", "", inidirValue, MAX_PATH, pathsIni) > 0) {
            if (strlen(inidirValue) > 0 && strstr(inidirValue, "{26}") == NULL) {
                validInidir = true;
            }
        }
    }

    if (validInidir) {
        sprintf(iniPath, "%s\\Plugins\\gen_eqhotkey.ini", inidirValue);
    } else {
        char appDataPath[MAX_PATH] = {0};
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appDataPath))) {
            sprintf(iniPath, "%s\\Winamp\\Plugins\\gen_eqhotkey.ini", appDataPath);
        }
    }
}

void SaveHotkey() {
    WritePrivateProfileInt("Hotkey", "Modifiers", currentHotkeyModifiers, iniPath);
    WritePrivateProfileInt("Hotkey", "Key", currentHotkeyKey, iniPath);
}

void LoadHotkey() {
    currentHotkeyModifiers = GetPrivateProfileInt("Hotkey", "Modifiers", MOD_CONTROL | MOD_SHIFT, iniPath);
    currentHotkeyKey       = GetPrivateProfileInt("Hotkey", "Key", 0x45, iniPath);
}

#define EQ_ENABLE 40244
void toggleWinampEQ() {
    if (!hwndWinamp) hwndWinamp = FindWindow("Winamp v1.x", NULL);
    if (hwndWinamp) {
        SendMessage(hwndWinamp, WM_COMMAND, EQ_ENABLE, 0);
    }
}

LRESULT CALLBACK HiddenWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_HOTKEY:
            if (wParam == HOTKEY_ID) toggleWinampEQ();
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

bool InitHiddenWindow() {
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = HiddenWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "WinampHotkeyEQClass";

    if (!RegisterClassEx(&wc)) return false;

    hwndHidden = CreateWindowEx(
        0, wc.lpszClassName, "Winamp EQ Hotkey Handler",
        0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL
    );
    return (hwndHidden != NULL);
}

bool RegisterGlobalHotkey() {
    if (hwndHidden) UnregisterHotKey(hwndHidden, HOTKEY_ID);
    if (RegisterHotKey(hwndHidden, HOTKEY_ID, currentHotkeyModifiers, currentHotkeyKey)) {
        return true;
    } else {
        hwndHidden = NULL;
        return false;
    }
}

void CenterWindow(HWND hwnd) {
    RECT rc; GetWindowRect(hwnd, &rc);
    int winW = rc.right - rc.left;
    int winH = rc.bottom - rc.top;
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int x = (screenW - winW) / 2;
    int y = (screenH - winH) / 2;
    SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

void AdoptWinampIcon(HWND hWnd) {
    if (!hwndWinamp) hwndWinamp = FindWindow("Winamp v1.x", NULL);
    if (!hwndWinamp) return;
    HICON hBig = (HICON)SendMessage(hwndWinamp, WM_GETICON, ICON_BIG, 0);
    HICON hSmall = (HICON)SendMessage(hwndWinamp, WM_GETICON, ICON_SMALL, 0);
    if (!hBig)   hBig   = (HICON)GetClassLongPtr(hwndWinamp, GCLP_HICON);
    if (!hSmall) hSmall = (HICON)GetClassLongPtr(hwndWinamp, GCLP_HICONSM);
    if (hBig)   SendMessage(hWnd, WM_SETICON, ICON_BIG,   (LPARAM)hBig);
    if (hSmall) SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hSmall);
}

HWND hHotkeyCtrl;
LRESULT CALLBACK ConfigWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HFONT hFont, hFontBig;
    switch (msg) {
        case WM_CREATE: {
            INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_WIN95_CLASSES };
            InitCommonControlsEx(&icc);

            NONCLIENTMETRICS ncm = { sizeof(NONCLIENTMETRICS) };
            SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
            hFont = CreateFontIndirect(&ncm.lfMessageFont);

            LOGFONT lfBig = ncm.lfMessageFont;
            hFontBig = CreateFontIndirect(&lfBig);

            int margin = 10;
            int y = margin;

            RECT rcClient;
            GetClientRect(hwnd, &rcClient);

            HWND hInstr = CreateWindow(
                "STATIC",
                "This plugin lets you set a quick shortcut to toggle the EQ on and off.\n\nTo change the default shortcut, enter your preferred key combination here and click OK:",
                WS_VISIBLE | WS_CHILD | SS_CENTER | SS_NOPREFIX,
                margin, y, rcClient.right - 2*margin, 90,
                hwnd, NULL, hInstance, NULL
            );
            SendMessage(hInstr, WM_SETFONT, (WPARAM)hFontBig, TRUE);
            y += 95;

            int hotkeyWidth = 180;
            hHotkeyCtrl = CreateWindowEx(0, HOTKEY_CLASS, "",
                                        WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
                                        (rcClient.right - hotkeyWidth)/2, y, hotkeyWidth, 22,
                                        hwnd, (HMENU)100, hInstance, NULL);
            WORD mods =
                (currentHotkeyModifiers & MOD_SHIFT   ? HOTKEYF_SHIFT   : 0) |
                (currentHotkeyModifiers & MOD_CONTROL ? HOTKEYF_CONTROL : 0) |
                (currentHotkeyModifiers & MOD_ALT     ? HOTKEYF_ALT     : 0);
            SendMessage(hHotkeyCtrl, HKM_SETHOTKEY, MAKEWORD(currentHotkeyKey, mods), 0);
            SendMessage(hHotkeyCtrl, WM_SETFONT, (WPARAM)hFont, TRUE);
            y += 35;

            HWND hInfo1 = CreateWindow(
                "STATIC",
                "EQ Toggle Hotkey v1.4.2\nBuild date: 2025/08/21\nWritten by: Daniele Borghi",
                WS_VISIBLE | WS_CHILD | SS_CENTER,
                margin, y, rcClient.right - 2*margin, 60,
                hwnd, NULL, hInstance, NULL
            );
            SendMessage(hInfo1, WM_SETFONT, (WPARAM)hFont, TRUE);
            y += 65;

            HWND hInfo2 = CreateWindow(
                "STATIC",
                "For more information and to access the source code, visit my GitHub page:\n\n https://github.com/danjiss",
                WS_VISIBLE | WS_CHILD | SS_CENTER,
                margin, y, rcClient.right - 2*margin, 60,
                hwnd, NULL, hInstance, NULL
            );
            SendMessage(hInfo2, WM_SETFONT, (WPARAM)hFont, TRUE);
            y += 65;

            HWND hOk = CreateWindow("BUTTON", "OK",
                                    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | WS_TABSTOP,
                                    (rcClient.right - 80)/2, rcClient.bottom - 40, 80, 24,
                                    hwnd, (HMENU)IDOK, hInstance, NULL);
            SendMessage(hOk, WM_SETFONT, (WPARAM)hFont, TRUE);
            break;
        }

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK) {
                LRESULT hk = SendMessage(hHotkeyCtrl, HKM_GETHOTKEY, 0, 0);
                BYTE vk = LOBYTE(hk);
                BYTE fmods = HIBYTE(hk);

                currentHotkeyKey = vk;
                currentHotkeyModifiers = 0;
                if (fmods & HOTKEYF_SHIFT)   currentHotkeyModifiers |= MOD_SHIFT;
                if (fmods & HOTKEYF_CONTROL) currentHotkeyModifiers |= MOD_CONTROL;
                if (fmods & HOTKEYF_ALT)     currentHotkeyModifiers |= MOD_ALT;

                RegisterGlobalHotkey();
                SaveHotkey();
                DestroyWindow(hwnd);
                return 0;
            }
            break;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;

        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                DestroyWindow(hwnd);
                return 0;
            }
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

const char pluginDesc[] = "Danji EQ Toggle Hotkey v1.4.2";

extern "C" __declspec(dllexport) winampGeneralPurposePlugin* winampGetGeneralPurposePlugin();
extern "C" __declspec(dllexport) int init();
extern "C" __declspec(dllexport) void config();
extern "C" __declspec(dllexport) void quit();

winampGeneralPurposePlugin plugin = {
    GPPHDR_VER,
    (char*)pluginDesc,
    init,
    config,
    quit,
    0,
    0
};

extern "C" __declspec(dllexport) winampGeneralPurposePlugin* winampGetGeneralPurposePlugin() {
    return &plugin;
}

int init() {
    hwndWinamp = FindWindow("Winamp v1.x", NULL);
    InitIniPath();
    LoadHotkey();
    if (!InitHiddenWindow()) return 1;
    RegisterGlobalHotkey();
    return 0;
}

void config() {
    WNDCLASS wc = {0};
    wc.lpfnWndProc   = ConfigWndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = "ConfigEQHotkey";
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    RegisterClass(&wc);

    RECT rc = {0, 0, 290, 350};
    AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, FALSE);

    HWND hwndCfg = CreateWindow(
        "ConfigEQHotkey", "EQ Hotkey Config",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        NULL, NULL, hInstance, NULL
    );

    AdoptWinampIcon(hwndCfg);
    CenterWindow(hwndCfg);
    ShowWindow(hwndCfg, SW_SHOW);
    UpdateWindow(hwndCfg);

    MSG msg;
    while (IsWindow(hwndCfg) && GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void quit() {
    if (hwndHidden) {
        UnregisterHotKey(hwndHidden, HOTKEY_ID);
        DestroyWindow(hwndHidden);
        hwndHidden = NULL;
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) hInstance = hModule;
    return TRUE;
}
