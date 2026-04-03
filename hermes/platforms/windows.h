#ifndef HERMES_PLATFORM_WINDOWS_H_
#define HERMES_PLATFORM_WINDOWS_H_


#ifdef __cplusplus
extern "C" {
#endif


#ifdef UI_WINDOWS
# undef _UNICODE
# undef UNICODE
# include <windows.h>


typedef struct Hermes_Platform_Windows {
    HCURSOR cursors[UI_CURSOR_COUNT];
    bool    assertionFailure;
} Hermes_Platform_Windows;


typedef struct Hermes_PlatformWindow_Windows {
    HWND hwnd;
    bool trackingLeave;
} Hermes_PlatformWindow_Windows;
#endif

#ifdef __cplusplus
}
#endif


#endif // HERMES_PLATFORM_WINDOWS_H_
