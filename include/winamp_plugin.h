#ifndef WINAMP_PLUGIN_H
#define WINAMP_PLUGIN_H

#define GPPHDR_VER 0x10

#include <windows.h>
#include <commctrl.h>

typedef struct {
    int version;
    char *description;
    int (*init)();
    void (*config)();
    void (*quit)();
    HWND hwndParent;
    HINSTANCE hDllInstance;
} winampGeneralPurposePlugin;

#endif
