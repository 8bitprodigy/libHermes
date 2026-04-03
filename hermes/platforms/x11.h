#ifndef HERMES_PLATFORM_X11_H_
#define HERMES_PLATFORM_X11_H_


#ifdef __cplusplus
extern "C" {
#endif


#ifdef UI_LINUX
# include "../platform.h"
# include "../ui_cursor.h"
# include <X11/Xatom.h>
# include <X11/Xlib.h>
# include <X11/Xutil.h>
# include <X11/cursorfont.h>
# include <stdbool.h>
# include <sys/epoll.h>


typedef struct Hermes_Platform_X11 {
    Display *display;
    Visual  *visual;
    XIM      xim;
    Atom     windowClosedID, primaryID, uriListID, plainTextID;
    Atom     dndEnterID, dndLeaveID, dndTypeListID, dndPositionID, dndStatusID, dndActionCopyID,
        dndDropID, dndSelectionID, dndFinishedID, dndAwareID;
    Atom   clipboardID, xSelectionDataID, textID, targetID, incrID;
    Cursor cursors[UI_CURSOR_COUNT];
    char  *pasteText;
    XEvent copyEvent;
    int    epollFD;
} Hermes_Platform_X11;


typedef struct Hermes_PlatformWindow_Data_X11 {
    XImage  *image;
    XIC      xic;
    unsigned ctrlCode, shiftCode, altCode;
    Window   dragSource, dragDestination;
    int      dragDestinationVersion;
    bool     inDrag, dragDestinationCanDrop;
    char    *uriList;
} Hermes_PlatformWindow_Data_X11_t;


typedef struct Hermes_PlatformWindow_X11 {
    Window window;
    void  *usr_ptr;
} Hermes_PlatformWindow_X11;
#endif // UI_LINUX

#ifdef __cplusplus
}
#endif


#endif // HERMES_PLATFORM_X11_H_
