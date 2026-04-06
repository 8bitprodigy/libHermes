#ifndef HERMES_PLATFORM_X11_H_
#define HERMES_PLATFORM_X11_H_


#ifdef __cplusplus
extern "C" {
#endif


#ifdef UI_LINUX
# include "ui_cursor.h"

# include <X11/Xatom.h>
# include <X11/Xlib.h>
# include <X11/Xutil.h>
# include <X11/cursorfont.h>
# include <assert.h>
# include <stdbool.h>
# include <stdlib.h>
# include <string.h>
# include <sys/epoll.h>
# include <time.h>


/* Memory */
# define UI_ASSERT       assert
# define UI_CALLOC(x)    calloc(1, (x))
# define UI_FREE         free
# define UI_MALLOC       malloc
# define UI_REALLOC      realloc
# define UI_MEMMOVE(d, s, n) memmove((d), (s), (n))


/* Clock */
# define UI_CLOCK             _UIClock
# define UI_CLOCKS_PER_SECOND 1000
# define UI_CLOCK_T           uint64_t
 
static inline UI_CLOCK_T _UIClock(void)
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    return (uint64_t)spec.tv_sec * 1000 + spec.tv_nsec / 1000000;
}


typedef struct 
Hermes_Platform_X11 
{
    Display *display;
    Visual  *visual;
    XIM      xim;
    Atom     
        windowClosedID, 
        primaryID, 
        uriListID, 
        plainTextID,  
        dndEnterID, 
        dndLeaveID, 
        dndTypeListID, 
        dndPositionID, 
        dndStatusID, 
        dndActionCopyID,
        dndDropID, 
        dndSelectionID, 
        dndFinishedID, 
        dndAwareID,
        clipboardID, 
        xSelectionDataID, 
        textID, 
        targetID, 
        incrID;
    Cursor  cursors[UI_CURSOR_COUNT];
    char   *pasteText;
    XEvent  copyEvent;
    int     epollFD;
} 
Hermes_Platform_X11;


typedef struct
HermesPlatformWindow
{
    Window  window;
    XImage *image;
    XIC     xic;
    unsigned  
        ctrlCode, 
        shiftCode, 
        altCode;
    Window    
        dragSource, 
        dragDestination;
    int dragDestinationVersion;
    bool      
        inDrag, 
        dragDestinationCanDrop;
    char *uriList;
} Hermes_PlatformWindow;

/*
typedef struct 
Hermes_PlatformWindow_Data_X11 
{
    XImage *image;
    XIC     xic;
    unsigned 
        ctrlCode, 
        shiftCode, 
        altCode;
    Window   
        dragSource, 
        dragDestination;
    int   dragDestinationVersion;
    bool  inDrag, dragDestinationCanDrop;
    char *uriList;
} 
Hermes_PlatformWindow_Data_X11_t;


typedef struct 
Hermes_PlatformWindow_X11 
{
    Window  window;
    void   *usr_ptr;
} 
Hermes_PlatformWindow_X11;
*/
#endif // UI_LINUX

#ifdef __cplusplus
}
#endif


#endif // HERMES_PLATFORM_X11_H_
