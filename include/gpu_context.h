#ifndef HERMES_GPU_CONTEXT_H
#define HERMES_GPU_CONTEXT_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "ui_rect.h"

#include <stdint.h>


/* Vertex */
typedef struct
HermesVertex
{
	float
		x, y, // Screen-space position
		u, v; // Texture coordinates (0,0 for solid color draws)
	uint32_t color; // ARGB
}
HermesVertex;


/* Texture */
typedef struct
HermesGPUTexture
{
	void *handle; // Backend-specific texture handle
	int
		width,
		height;
}
HermesGPUTexture;


/* GPU Context */
typedef struct
HermesGPUContext
{
	/* Primitive Drawing */
	void (*fill_rect )(struct HermesGPUContext *, UIRectangle, uint32_t color);
    void (*draw_line )(struct HermesGPUContext *, int x0, int y0, int x1, int y1, uint32_t color);
    void (*draw_tris )(struct HermesGPUContext *, HermesVertex *verts, int count);
    void (*draw_image)(struct HermesGPUContext *, UIRectangle   dst, UIRectangle src, HermesGPUTexture *texture);

    /* Clip */
    void (*set_clip     )(struct HermesGPUContext *, UIRectangle);
    void (*restore_clip )(struct HermesGPUContext *);

    /* Texture Management */
    HermesGPUTexture *(*upload_texture)(struct HermesGPUContext *, uint32_t *bits, int w, int h);
    void              (*free_texture  )(struct HermesGPUContext *, HermesGPUTexture *texture);
    void              (*set_tint      )(struct HermesGPUContext *, HermesGPUTexture *, uint32_t argb);

    /* Frame */
    void (*begin  )(struct HermesGPUContext *);
    void (*present)(struct HermesGPUContext *);

    /* Escape Hatch */
    void *(*get_native)(struct HermesGPUContext *);
 
    void *ctx; // backend-specific data
}
HermesGPUContext;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* HERMES_GPU_CONTEXT_H */
