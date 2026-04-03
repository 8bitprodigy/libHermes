#ifndef HERMES_SHORTCUT_H_
#define HERMES_SHORTCUT_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <stdbool.h>
#include <stdint.h>


#define UI_SHORTCUT(code, ctrl, shift, alt, invoke, cp)                                            \
    ((UIShortcut){(code), (ctrl), (shift), (alt), (invoke), (cp)})


typedef struct UIShortcut {
    intptr_t code;
    bool     ctrl, shift, alt;
    void (*invoke)(void *cp);
    void *cp;
} UIShortcut;


#ifdef __cplusplus
}
#endif


#endif // HERMES_SHORTCUT_H_
