#ifndef HERMES_PLATFORM_ESSENCE_H_
#define HERMES_PLATFORM_ESSENCE_H_


#ifdef __cplusplus
extern "C" {
#endif


#ifdef UI_ESSENCE
typedef struct Hermes_Platform_Essence {
    EsInstance *instance;
    void       *menuData[256]; // HACK This limits the number of menu items to 128.
    uintptr_t   menuIndex;
} Hermes_Platform_Essence;


typedef struct Hermes_PlatformWindow_Essence {
    EsWindow  *window;
    EsElement *canvas;
    int        cursor;
} Hermes_PlatformWindow_Essence;
#endif


#ifdef __cplusplus
}
#endif


#endif // HERMES_PLATFORM_ESSENCE_H_
