#include "sdl3_gpu_context.h"


#ifdef UI_SDL3


#include "ui_rect.h"
#include "utils.h"

#include <SDL3/SDL.h>
#include <stdint.h>


/* -------------------------------------------------------------------------
 * Helpers
 * ---------------------------------------------------------------------- */

static inline SDL3RendererCtx *
_SDLCtx(HermesGPUContext *gpu)
{
    return (SDL3RendererCtx *)gpu->ctx;
}

/*
 * Hermes stores colors as 0xAARRGGBB (ARGB, host-endian uint32).
 * SDL_SetRenderDrawColor wants separate R, G, B, A bytes.
 */
static inline void
_SDLSetColor(SDL_Renderer *renderer, uint32_t argb)
{
    uint8_t a = (argb >> 24) & 0xFF;
    uint8_t r = (argb >> 16) & 0xFF;
    uint8_t g = (argb >>  8) & 0xFF;
    uint8_t b = (argb      ) & 0xFF;
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

static inline SDL_FRect
_SDLFRect(UIRectangle r)
{
    SDL_FRect fr;
    fr.x = (float)r.l;
    fr.y = (float)r.t;
    fr.w = (float)(r.r - r.l);
    fr.h = (float)(r.b - r.t);
    return fr;
}


/* -------------------------------------------------------------------------
 * VTable implementations
 * ---------------------------------------------------------------------- */

static void
_SDL3_fill_rect(HermesGPUContext *gpu, UIRectangle r, uint32_t color)
{
    SDL_Renderer *renderer = _SDLCtx(gpu)->renderer;
    _SDLSetColor(renderer, color);
    SDL_FRect fr = _SDLFRect(r);
    SDL_RenderFillRect(renderer, &fr);
}


static void
_SDL3_draw_line(HermesGPUContext *gpu, int x0, int y0, int x1, int y1, uint32_t color)
{
    SDL_Renderer *renderer = _SDLCtx(gpu)->renderer;
    _SDLSetColor(renderer, color);
    SDL_RenderLine(renderer, (float)x0, (float)y0, (float)x1, (float)y1);
}


static void
_SDL3_draw_tris(HermesGPUContext *gpu, HermesVertex *verts, int count)
{
    SDL_Renderer *renderer = _SDLCtx(gpu)->renderer;

    /*
     * SDL_RenderGeometry takes SDL_Vertex[], which has separate x/y/r/g/b/a
     * fields.  We convert from HermesVertex on the stack.  For the common case
     * of a single triangle (count == 3) this is zero-allocation.
     *
     * For solid-color draws, uv is (0,0) and there is no texture — we pass
     * NULL for the texture argument.
     *
     * Textured draws go through draw_image, not draw_tris, so we never need
     * to look up a texture here.
     */

    /* Stack buffer for the common case; fall back to heap for larger batches. */
    SDL_Vertex  stack_buf[128];
    SDL_Vertex *sv = (count <= 128) ? stack_buf : (SDL_Vertex *)UI_MALLOC(sizeof(SDL_Vertex) * count);
    if (!sv) return;

    for (int i = 0; i < count; i++) {
        sv[i].position.x = verts[i].x;
        sv[i].position.y = verts[i].y;
        sv[i].tex_coord.x = verts[i].u;
        sv[i].tex_coord.y = verts[i].v;

        uint32_t argb    = verts[i].color;
        sv[i].color.r    = (float)((argb >> 16) & 0xFF) / 255.0f;
        sv[i].color.g    = (float)((argb >>  8) & 0xFF) / 255.0f;
        sv[i].color.b    = (float)((argb      ) & 0xFF) / 255.0f;
        sv[i].color.a    = (float)((argb >> 24) & 0xFF) / 255.0f;
    }

    SDL_RenderGeometry(renderer, NULL, sv, count, NULL, 0);

    if (sv != stack_buf)
        UI_FREE(sv);
}


static void
_SDL3_draw_image(HermesGPUContext *gpu, UIRectangle dst, UIRectangle src, HermesGPUTexture *texture)
{
    SDL_Renderer *renderer = _SDLCtx(gpu)->renderer;
    SDL_Texture  *sdl_tex  = (SDL_Texture *)texture->handle;
    SDL_FRect dst_rect = _SDLFRect(dst);
    SDL_FRect src_rect = _SDLFRect(src);
    SDL_RenderTexture(renderer, sdl_tex, &src_rect, &dst_rect);
}


static void
_SDL3_set_clip(HermesGPUContext *gpu, UIRectangle r)
{
    SDL_Renderer *renderer = _SDLCtx(gpu)->renderer;
    SDL_Rect clip;
    clip.x = r.l;
    clip.y = r.t;
    clip.w = r.r - r.l;
    clip.h = r.b - r.t;
    SDL_SetRenderClipRect(renderer, &clip);
}

static void
_SDL3_restore_clip(HermesGPUContext *gpu)
{
    /*
     * The painter maintains the clip stack and calls set_clip with the
     * restored rectangle after popping.  This function is a no-op here
     * because the actual scissor state is updated by the following set_clip
     * call that the painter always issues immediately after restore_clip.
     *
     * If you ever call restore_clip without a matching set_clip (i.e., back
     * to "no clip"), pass NULL to SDL_SetRenderClipRect to disable clipping.
     */
    (void)gpu;
}


static void
_SDL3_set_tint(HermesGPUContext *gpu, HermesGPUTexture *texture, uint32_t argb)
{
    (void)gpu;
    SDL_Texture *sdl_tex = (SDL_Texture *)texture->handle;
    SDL_SetTextureColorModFloat(sdl_tex,
        (float)((argb >> 16) & 0xFF) / 255.0f,
        (float)((argb >>  8) & 0xFF) / 255.0f,
        (float)((argb      ) & 0xFF) / 255.0f);
    SDL_SetTextureAlphaModFloat(sdl_tex,
        (float)((argb >> 24) & 0xFF) / 255.0f);
}


static HermesGPUTexture *
_SDL3_upload_texture(HermesGPUContext *gpu, uint32_t *bits, int w, int h)
{
    SDL_Renderer *renderer = _SDLCtx(gpu)->renderer;

    /*
     * SDL_PIXELFORMAT_ARGB8888 matches Hermes's 0xAARRGGBB uint32 layout on
     * little-endian systems (which is the only layout Hermes targets today).
     * The glyph stores alpha in the high byte and white in RGB, so
     * ARGB8888 reads it correctly.
     */
    SDL_Surface *surface = SDL_CreateSurfaceFrom(
        w, h,
        SDL_PIXELFORMAT_ARGB8888,
        bits,
        w * 4
    );
    if (!surface) return NULL;

    SDL_Texture *sdl_tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    if (!sdl_tex) return NULL;

    /*
     * Enable alpha blending so glyph atlas pixels blend over the background
     * rather than replacing it.
     */
    SDL_SetTextureBlendMode(sdl_tex, SDL_BLENDMODE_BLEND);

    HermesGPUTexture *tex = (HermesGPUTexture *)UI_MALLOC(sizeof(HermesGPUTexture));
    if (!tex) {
        SDL_DestroyTexture(sdl_tex);
        return NULL;
    }

    tex->handle = sdl_tex;
    tex->width  = w;
    tex->height = h;
    return tex;
}

static void
_SDL3_free_texture(HermesGPUContext *gpu, HermesGPUTexture *texture)
{
    (void)gpu;
    if (!texture) return;
    SDL_DestroyTexture((SDL_Texture *)texture->handle);
    UI_FREE(texture);
}


static void
_SDL3_begin(HermesGPUContext *gpu)
{
    SDL_Renderer *renderer = _SDLCtx(gpu)->renderer;
    /* Clear with full transparency so the window background shows through
       if the compositor supports it; otherwise it will be black. */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    /* Restore normal alpha blending for subsequent draw calls. */
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
}


static void
_SDL3_present(HermesGPUContext *gpu)
{
    SDL_RenderPresent(_SDLCtx(gpu)->renderer);
}


static void *
_SDL3_get_native(HermesGPUContext *gpu)
{
    return (void *)_SDLCtx(gpu)->renderer;
}


/* -------------------------------------------------------------------------
 * Constructor / Destructor
 * ---------------------------------------------------------------------- */

HermesGPUContext *
SDL3GPUContext_create(SDL_Window *window)
{
    SDL3RendererCtx *sctx = (SDL3RendererCtx *)UI_MALLOC(sizeof(SDL3RendererCtx));
    if (!sctx) return NULL;

    /*
     * NULL driver name lets SDL pick the best available backend (Metal on
     * macOS, D3D12 on Windows, Vulkan/OpenGL on Linux).
     */
    sctx->renderer = SDL_CreateRenderer(window, NULL);
    if (!sctx->renderer) {
        UI_FREE(sctx);
        return NULL;
    }
    sctx->window = window;

    HermesGPUContext *gpu = (HermesGPUContext *)UI_MALLOC(sizeof(HermesGPUContext));
    if (!gpu) {
        SDL_DestroyRenderer(sctx->renderer);
        UI_FREE(sctx);
        return NULL;
    }

    gpu->fill_rect       = _SDL3_fill_rect;
    gpu->draw_line       = _SDL3_draw_line;
    gpu->draw_tris       = _SDL3_draw_tris;
    gpu->draw_image      = _SDL3_draw_image;
    gpu->set_clip        = _SDL3_set_clip;
    gpu->restore_clip    = _SDL3_restore_clip;
    gpu->set_tint        = _SDL3_set_tint;
    gpu->upload_texture  = _SDL3_upload_texture;
    gpu->free_texture    = _SDL3_free_texture;
    gpu->begin           = _SDL3_begin;
    gpu->present         = _SDL3_present;
    gpu->get_native      = _SDL3_get_native;
    gpu->ctx             = sctx;

    return gpu;
}


void
SDL3GPUContext_destroy(HermesGPUContext *gpu)
{
    if (!gpu) return;
    SDL3RendererCtx *sctx = _SDLCtx(gpu);
    SDL_DestroyRenderer(sctx->renderer);
    UI_FREE(sctx);
    UI_FREE(gpu);
}


#endif /* UI_SDL3 */
