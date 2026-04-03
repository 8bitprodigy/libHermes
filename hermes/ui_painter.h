#ifndef HERMES_PAINTER_H_
#define HERMES_PAINTER_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_rect.h"
#include <stddef.h>
#include <stdint.h>


typedef struct UIPainter {
    void (*draw_block  )(struct UIPainter *, UIRectangle, uint32_t color);
    void (*draw_line   )(struct UIPainter *, int x0, int y0, int x1, int y1, uint32_t color);
    void (*draw_glyph  )(struct UIPainter *, int  x, int  y, int  c, uint32_t color);
    void (*draw_invert )(struct UIPainter *, UIRectangle);
    void (*set_clip    )(struct UIPainter *, UIRectangle);
    void (*restore_clip)(struct UIPainter *);
    
    void        *ctx;
    UIRectangle  clip;
    uint32_t    *bits;
    int         
        width, 
        height;
#ifdef UI_DEBUG
    int fillCount;
#endif
} UIPainter;


#ifdef __cplusplus
}
#endif

#endif // HERMES_PAINTER_H_
