#ifndef LUIGI_PLATFORM_H_
#define LUIGI_PLATFORM_H_


#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif


#ifdef UI_LINUX
    #include "../hermes/platforms/x11.h"
#endif
#ifdef UI_COCOA
    #include "../hermes/platforms/cocoa.h"
#endif
#ifdef UI_WINDOWS
    #include "../hermes/platforms/windows.h"
#endif
#ifdef UI_SDL3
    #include "../hermes/platforms/sdl3.h"
#endif


#include "ui_painter.h"


//


typedef void * Hermes_Platform;

typedef struct UIWindow UIWindow;
typedef struct UIMenu   UIMenu;


 Hermes_Platform * Hermes_PlatformInit(void);

void  Hermes_Platform_CreateWindow(UIWindow *window, uint32_t flags, const char *cTitle, int _width, int _height);
void  Hermes_Platform_DestroyWindow( Hermes_PlatformWindow *window);

void  Hermes_Platform_get_screen_pos( Hermes_PlatformWindow *pwindow, int *_x, int *_y);
void  Hermes_Platform_render(UIWindow *window, UIPainter *painter);
void  _UIWindowSetCursor(UIWindow *window, int cursor);
const char *Hermes_GetBackendName(void);


void  UIMenuShow(UIMenu *menu);


//


#ifdef UI_LINUX
UIWindow *_UIFindWindow(Window window);
#endif
#ifdef UI_WINDOWS
void *_UIHeapReAlloc(void *pointer, size_t size);
void *_UIMemmove(void *dest, const void *src, size_t n);
#endif


#ifdef __cplusplus
}
#endif


#endif // LUIGI_PLATFORM_H_
