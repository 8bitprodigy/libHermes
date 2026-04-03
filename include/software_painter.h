#ifndef HERMES_SOFTWARE_PAINTER_H_
#define HERMES_SOFTWARE_PAINTER_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_painter.h"
#include "ui_string.h"


void _SWDrawBlock( UIPainter *painter, UIRectangle rectangle, uint32_t color);
void _SWDrawCircle(
     UIPainter *painter, 
     int        cx, 
     int        cy, 
     int        radius, 
     uint32_t   fillColor,
     uint32_t   outlineColor, 
     bool       hollow
);
void _SWDrawImage( UIPainter *painter, UIRectangle dst, uint32_t *src_bits, int src_width, int src_height);
void _SWDrawInvert(UIPainter *painter, UIRectangle rectangle);
void _SWDrawLine(
     UIPainter *painter, 
     int        x0, 
     int        y0, 
     int        x1, 
     int        y1, 
     uint32_t   color
);
void _SWDrawTriangle(
     UIPainter *painter, 
     int        x0, 
     int        y0, 
     int        x1, 
     int        y1, 
     int        x2, 
     int        y2,
     uint32_t   color
);

UIPainter SWPainter_create(uint32_t *bits, int width, int height);

#ifdef __cplusplus
}
#endif


#endif // HERMES_SOFTWARE_PAINTER_H_
