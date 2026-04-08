#ifndef HERMES_SDL3_GPU_CONTEXT_H_
#define HERMES_SDL3_GPU_CONTEXT_H_


#ifdef __cplusplus
extern "C" {
#endif


#ifdef UI_SDL3

#include "gpu_context.h"
#include "ui_rect.h"

#include <SDL3/SDL.h>
#include <stdint.h>


/*
 * SDL3RendererCtx
 *
 * Backend-specific data stored in HermesGPUContext.ctx.
 * The renderer is owned externally (by the window) and must outlive this context.
 */
typedef struct
SDL3RendererCtx
{
    SDL_Renderer *renderer;
    SDL_Window   *window;
}
SDL3RendererCtx;


/*
 * SDL3GPUContext_create
 *
 * Creates a HermesGPUContext backed by an SDL_Renderer.
 * The renderer is created from the given window and is owned by the returned context.
 * Returns NULL on failure.
 */
HermesGPUContext *SDL3GPUContext_create(SDL_Window *window);

/*
 * SDL3GPUContext_destroy
 *
 * Destroys the renderer and frees all resources associated with the context.
 * Do not call GPUPainter_destroy after this — destroy the painter first.
 */
void SDL3GPUContext_destroy(HermesGPUContext *ctx);


#endif /* UI_SDL3 */


#ifdef __cplusplus
}
#endif


#endif /* HERMES_SDL3_GPU_CONTEXT_H_ */
