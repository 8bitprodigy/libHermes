#ifndef HERMES_UI_STRING_H_
#define HERMES_UI_STRING_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <stddef.h>
#include <stdint.h>


typedef struct UIStringSelection {
    int      carets[2];
    uint32_t colorText, colorBackground;
} UIStringSelection;


int UIMeasureStringWidth(const char *string, ptrdiff_t bytes);
int UIMeasureStringHeight(void);


#endif // HERMES_UI_STRING_H_
