#ifndef HERMES_PAINTER_H_
#define HERMES_PAINTER_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_rect.h"
#include <stddef.h>
#include <stdint.h>


typedef struct
{
    uint32_t *bits;
    int
        width,
        height;
}
SWPainterCtx;

typedef struct 
UIPainter 
{
    void  (*draw_block        )(struct UIPainter *, UIRectangle,                uint32_t color);
    void  (*draw_image        )(struct UIPainter *, UIRectangle bounds, uint32_t *bits, int width, int height);
    void  (*draw_line         )(struct UIPainter *, int x0, int y0, int     x1, int         y1, uint32_t   color);
    void  (*draw_circle       )(struct UIPainter *, int cx, int cy, int radius, uint32_t  fill, uint32_t outline, bool hollow);
    void  (*draw_triangle     )(struct UIPainter *, int x0, int y0, int     x1, int         y1, int           x2, int      y2, uint32_t color);
    void  (*draw_glyph        )(struct UIPainter *, int  x, int  y, int      c, uint32_t color);
    void  (*draw_invert       )(struct UIPainter *, UIRectangle);
    void  (*set_clip          )(struct UIPainter *, UIRectangle);
    void  (*restore_clip      )(struct UIPainter *);
    void *(*get_native_context)(struct UIPainter *);
    
    void        *ctx;
    UIRectangle *clip_stack;
    UIRectangle  clip;
#ifdef UI_DEBUG
    int   fillCount;
#endif
} UIPainter;


#ifdef __cplusplus
}
#endif

#endif // HERMES_PAINTER_H_
