#ifndef HERMES_GPU_PAINTER_H
#define HERMES_GPU_PAINTER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "gpu_context.h"
#include "ui_painter.h"
#include "ui_rect.h"

#include <stdbool.h>
#include <stdint.h>


/* GPU Painter Context */
typedef struct
GPUPainterCtx 
{
	HermesGPUContext *gpu;        // The underlying GPU context
	UIRectangle      *clip_stack; // DynamicArray of UIRectangle
	struct GlyphAtlas *atlas;     // Glyph atlas (owned)
	int 
		last_glyph_x,
		last_glyph_y,
		last_glyph_c;
}
GPUPainterCtx;


/* Painter VTable Implementations */
void  _GPUDrawBlock(   UIPainter *, UIRectangle, uint32_t color);
void  _GPUDrawLine(    UIPainter *, int x0, int y0, int x1, int y1, uint32_t color);
void  _GPUDrawCircle(  UIPainter *, int cx, int cy, int radius, uint32_t fill, uint32_t outline, bool hollow);
void  _GPUDrawTriangle(UIPainter *, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
void  _GPUDrawGlyph(   UIPainter *, int  x, int  y, int  c, uint32_t color);
void  _GPUDrawInvert(  UIPainter *, UIRectangle);
void  _GPUDrawImage(   UIPainter *, UIRectangle dst, uint32_t *bits, int w, int h);
void  _GPUSetClip(     UIPainter *, UIRectangle);
void  _GPURestoreClip( UIPainter *);
void *_GPUGetNative(   UIPainter *);


/* Constructor */
UIPainter GPUPainter_create(HermesGPUContext *gpu, int width, int height);
void      GPUPainter_destroy(UIPainter *painter);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* HERMES_GPU_PAINTER_H */
