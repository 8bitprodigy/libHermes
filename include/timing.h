#ifndef LUIGI_TIMING_H_
#define LUIGI_TIMING_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "platform.h"

/*
    Clock definitions (UI_CLOCK, UI_CLOCK_T, UI_CLOCKS_PER_SECOND) are defined
    in each platform header:
        platforms/x11.h     -- UI_LINUX
        platforms/windows.h -- UI_WINDOWS
        platforms/sdl3.h    -- UI_SDL3
        platforms/cocoa.h   -- UI_COCOA
*/


#ifdef __cplusplus
}
#endif


#endif // LUIGI_TIMING_H_
