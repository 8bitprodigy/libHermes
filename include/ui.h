#ifndef LUIGI_UI_H_
#define LUIGI_UI_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "font.h"
#include "platform.h"
#include "ui_element.h"
#include "ui_theme.h"
#include "ui_window.h"


typedef enum UI_Alignment {
    UI_ALIGN_LEFT = 1,
    UI_ALIGN_RIGHT,
    UI_ALIGN_CENTER,
} UI_Alignment;


//


struct        Hermes;
extern struct Hermes ui;


struct  
Hermes 
{
     Hermes_Platform platform;

#ifdef UI_FREETYPE
    FT_Library ft;
#endif

    UIWindow *windows;
    UITheme   theme;

    UIElement **animating;
    uint32_t    animatingCount;

    UIElement *parentStack[16];
    int        parentStackCount;

    bool        quit;
    const char *dialogResult;
    UIElement  *dialogOldFocus;
    bool        dialogCanExit;

    UIFont *activeFont;
};


//


typedef struct  
Hermes_InitConfig 
{
    bool with_inspector;
}  
Hermes_InitConfig;


//

void  Hermes_Init( Hermes_InitConfig *config);
void  Hermes_UpdateUI(void);
int   Hermes_Loop(void);


//


#ifdef __cplusplus
}
#endif


#endif // LUIGI_UI_H_
