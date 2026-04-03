#ifndef HERMES_PLATFORM_COCOA_H_
#define HERMES_PLATFORM_COCOA_H_


#ifdef __cplusplus
extern "C" {
#endif


#ifdef UI_COCOA
# import <Carbon/Carbon.h>
# import <Cocoa/Cocoa.h>
# import <Foundation/Foundation.h>


typedef struct Hermes_Platform_Cocoa {
    void     *menuData[256]; // HACK This limits the number of menu items to 128.
    uintptr_t menuIndex;
    int       menuX, menuY;
    UIWindow *menuWindow;
} Hermes_Platform_Cocoa;


typedef struct Hermes_PlatformWindow_Cocoa {
    NSWindow *window;
    void     *view;
} Hermes_PlatformWindow_Cocoa;
#endif


#ifdef __cplusplus
}
#endif


#endif // HERMES_PLATFORM_COCOA_H_
