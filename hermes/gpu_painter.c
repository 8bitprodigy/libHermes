#include "gpu_painter.h"
#include "font.h"
#include "ui.h"
#include "ui_rect.h"
#include "utils.h"
#include "dynamicarray.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>


/* Helpers */
static inline GPUPainterCtx *
_GPUCtx(UIPainter *painter)
{
	return (GPUPainterCtx*)painter->ctx;
}

static inline HermesGPUContext *
_GPU(UIPainter *painter)
{
	return _GPUCtx(painter)->gpu;
}

/* Convert uint32 XRGB color to ARGB with full alpha */
static inline uint32_t
_ColorWithAlpha(uint32_t color)
{
	return color | 0xFF000000;
}


/* Glyph Atlas */
#define GLYPH_ATLAS_WIDTH  512
#define GLYPH_ATLAS_HEIGHT 512


typedef struct
GlyphAtlasEntry
{
	UIRectangle bounds; /* Position in Atlas */
	int
		codepoint,
		offset_x,
		offset_y,
		advance;
	bool valid;
}
GlyphAtlasEntry;

typedef struct
GlyphAtlas
{
	HermesGPUTexture *texture;
	uint32_t         *bits;    // CPU-side atlas buffer
	GlyphAtlasEntry  *entries; // DynamicArray
	int
		cursor_x,
		cursor_y,
		row_height;
	bool dirty;                // Needs re-upload
}
GlyphAtlas;


static GlyphAtlas *
_GlyphAtlasCreate(void)
{
	GlyphAtlas *atlas = (GlyphAtlas *)UI_MALLOC(sizeof(GlyphAtlas));
	if (!atlas) return NULL;
	atlas->bits      = (uint32_t *)UI_CALLOC(GLYPH_ATLAS_WIDTH * GLYPH_ATLAS_HEIGHT * 4);
	atlas->entries   = (GlyphAtlasEntry *)DynamicArray(GlyphAtlasEntry, 256);
	atlas->texture   = NULL;
	atlas->cursor_x  = 0;
	atlas->cursor_y  = 0;
	atlas->row_height = 0;
	atlas->dirty     = true;
	return atlas;
}


static GlyphAtlasEntry *
_GlyphAtlasFind(GlyphAtlas *atlas, int codepoint)
{
	size_t len = DynamicArray_length(atlas->entries);
	for (size_t i = 0; i < len; i++) {
		if (atlas->entries[i].codepoint == codepoint && atlas->entries[i].valid)
			return &atlas->entries[i];
	}

	return NULL;
}

static GlyphAtlasEntry *
_GlyphAtlasAdd(GlyphAtlas *atlas, int codepoint)
{
#ifdef UI_FREETYPE
    UIFont *font = ui.activeFont;
    if (!font->isFreeType) goto fallback;
 
    UIEnsureGlyphRendered(font, codepoint);
    FT_Bitmap *bitmap = &font->glyphs[codepoint];
 
    int 
		gw = bitmap->width,
		gh = bitmap->rows;
 
    // Wrap to next row if needed
    if (atlas->cursor_x + gw > GLYPH_ATLAS_WIDTH) {
        atlas->cursor_x   = 0;
        atlas->cursor_y  += atlas->row_height + 1;
        atlas->row_height  = 0;
    }
 
    if (atlas->cursor_y + gh > GLYPH_ATLAS_HEIGHT) {
        // Atlas full — for now just return NULL
        // TODO: implement atlas eviction/growth
        return NULL;
    }
 
    // Copy glyph bitmap into atlas
    for (int y = 0; y < gh; y++) {
        for (int x = 0; x < gw; x++) {
            uint8_t alpha = 0;
            if (bitmap->pixel_mode == FT_PIXEL_MODE_GRAY) {
                alpha = ((uint8_t *)bitmap->buffer)[x + y * bitmap->pitch];
            } else if (bitmap->pixel_mode == FT_PIXEL_MODE_MONO) {
                alpha = (((uint8_t *)bitmap->buffer)[(x >> 3) + y * bitmap->pitch] &
                         (0x80 >> (x & 7))) ? 255 : 0;
            }
            atlas->bits[(atlas->cursor_y + y) * GLYPH_ATLAS_WIDTH + (atlas->cursor_x + x)] =
                (alpha << 24) | 0x00FFFFFF;
        }
    }
 
    GlyphAtlasEntry entry = {0};
    entry.codepoint = codepoint;
    entry.bounds    = UI_RECT_4(
        atlas->cursor_x, atlas->cursor_x + gw,
        atlas->cursor_y, atlas->cursor_y + gh
    );
    entry.offset_x  = font->glyphOffsetsX[codepoint];
    entry.offset_y  = font->glyphOffsetsY[codepoint];
    entry.advance   = font->glyphAdvance[codepoint];
    entry.valid     = true;
 
    atlas->cursor_x += gw + 1;
    if (gh > atlas->row_height) atlas->row_height = gh;
    atlas->dirty = true;
 
    DynamicArray_append((void **)&atlas->entries, &entry, 1);
    return &atlas->entries[DynamicArray_length(atlas->entries) - 1];
 
fallback:
#endif
    // Bitmap font fallback — 8x16 glyphs
    if (codepoint < 0 || codepoint > 127) codepoint = '?';
 
    gw = 8, gh = 16;
 
    if (atlas->cursor_x + gw > GLYPH_ATLAS_WIDTH) {
        atlas->cursor_x   = 0;
        atlas->cursor_y  += atlas->row_height + 1;
        atlas->row_height  = 0;
    }

    if (atlas->cursor_y + gh > GLYPH_ATLAS_HEIGHT) {
        // Atlas full — TODO: eviction/growth
        return NULL;
    }
 
    extern const uint64_t __default_font[];
    const uint8_t *data = (const uint8_t *)__default_font + codepoint * 16;
 
    for (int y = 0; y < gh; y++) {
        uint8_t byte = data[y];
        for (int x = 0; x < gw; x++) {
            uint8_t alpha = (byte & (1 << x)) ? 255 : 0;
            atlas->bits[(atlas->cursor_y + y) * GLYPH_ATLAS_WIDTH + (atlas->cursor_x + x)] =
                (alpha << 24) | 0x00FFFFFF;
        }
    }
 
    entry = (GlyphAtlasEntry){0};
    entry.codepoint = codepoint;
    entry.bounds    = UI_RECT_4(
        atlas->cursor_x, atlas->cursor_x + gw,
        atlas->cursor_y, atlas->cursor_y + gh
    );
    entry.offset_x  = 0;
    entry.offset_y  = 0;
    entry.advance   = 9;
    entry.valid     = true;
 
    atlas->cursor_x += gw + 1;
    if (gh > atlas->row_height) atlas->row_height = gh;
    atlas->dirty = true;
 
    DynamicArray_append((void **)&atlas->entries, &entry, 1);
    return &atlas->entries[DynamicArray_length(atlas->entries) - 1];
}


/* Painter VTable */

void
_GPUDrawBlock(UIPainter *painter, UIRectangle r, uint32_t color)
{
	r = UIRectangleIntersection(painter->clip, r);
	if (!UI_RECT_VALID(r)) return;
	_GPU(painter)->fill_rect(_GPU(painter), r, _ColorWithAlpha(color));
}


void
_GPUDrawLine(
	UIPainter *painter, 
	int x0, int y0, 
	int x1, int y1, 
	uint32_t   color
)
{
	_GPU(painter)->draw_line(
			_GPU(painter), 
			x0, y0, 
			x1, y1, 
			_ColorWithAlpha(color)
		);
}


void _GPUDrawTriangle(
	UIPainter *painter, 
	int x0, int y0, 
	int x1, int y1, 
	int x2, int y2, 
	uint32_t   color
)
{
	uint32_t c = _ColorWithAlpha(color);
	HermesVertex verts[3] = {
		{ (float)x0, (float)y0, 0, 0, c },
		{ (float)x1, (float)y1, 0, 0, c },
		{ (float)x2, (float)y2, 0, 0, c },
	};
	_GPU(painter)->draw_tris(_GPU(painter), verts, 3);
}
 
 
void 
_GPUDrawGlyph(
	UIPainter *painter, 
	int x, int y, 
	int c, 
	uint32_t   color
)
{
    GPUPainterCtx *ctx   = _GPUCtx(painter);
    GlyphAtlas    *atlas = ctx->atlas;
 
    GlyphAtlasEntry *entry = _GlyphAtlasFind(atlas, c);
    if (!entry) entry = _GlyphAtlasAdd(atlas, c);
    if (!entry) return;
 
    // Upload atlas texture if dirty
    if (atlas->dirty || !atlas->texture) {
        if (atlas->texture)
            ctx->gpu->free_texture(ctx->gpu, atlas->texture);
        atlas->texture = ctx->gpu->upload_texture(
            ctx->gpu, atlas->bits, GLYPH_ATLAS_WIDTH, GLYPH_ATLAS_HEIGHT);
        atlas->dirty = false;
    }
 
    UIRectangle dst = UI_RECT_4(
        x + entry->offset_x,
        x + entry->offset_x + UI_RECT_WIDTH(entry->bounds),
        y + entry->offset_y,
        y + entry->offset_y + UI_RECT_HEIGHT(entry->bounds)
    );
    
    ctx->gpu->set_tint(  ctx->gpu, atlas->texture, _ColorWithAlpha(color));
    ctx->gpu->draw_image(ctx->gpu, dst,            entry->bounds, atlas->texture);
    ctx->gpu->set_tint(  ctx->gpu, atlas->texture, 0xFFFFFFFF);
}


void
_GPUDrawImage(UIPainter *painter, UIRectangle dst, uint32_t *bits, int w, int h)
{
    dst = UIRectangleIntersection(painter->clip, dst);
    if (!UI_RECT_VALID(dst)) return;
    // TODO: cache uploaded textures rather than uploading every frame
    HermesGPUTexture *tex = _GPU(painter)->upload_texture(_GPU(painter), bits, w, h);
    if (!tex) return;
    UIRectangle src = UI_RECT_4(0, w, 0, h);
    _GPU(painter)->draw_image(_GPU(painter), dst, src, tex);
    _GPU(painter)->free_texture(_GPU(painter), tex);
}


void
_GPUDrawInvert(UIPainter *painter, UIRectangle r)
{
    r = UIRectangleIntersection(painter->clip, r);
    if (!UI_RECT_VALID(r)) return;
    // TODO: proper XOR invert requires a custom blend mode via SDL_ComposeCustomBlendMode
    _GPU(painter)->fill_rect(_GPU(painter), r, 0x80FFFFFF);
}


void
_GPUDrawCircle(UIPainter *painter, int cx, int cy, int radius, uint32_t fill, uint32_t outline, bool hollow)
{
    // TODO: hollow and outline not yet handled
    (void)outline; (void)hollow;
    
    #define CIRCLE_SEGMENTS 32
    uint32_t c = _ColorWithAlpha(fill);
    HermesVertex verts[CIRCLE_SEGMENTS * 3];
    
    for (int i = 0; i < CIRCLE_SEGMENTS; i++) {
        float a0 = (float)i       / CIRCLE_SEGMENTS * 2.0f * 3.14159265f;
        float a1 = (float)(i + 1) / CIRCLE_SEGMENTS * 2.0f * 3.14159265f;
        verts[i * 3 + 0] = (HermesVertex){ (float)cx,                          (float)cy,                          0, 0, c };
        verts[i * 3 + 1] = (HermesVertex){ (float)cx + cosf(a0) * radius,      (float)cy + sinf(a0) * radius,      0, 0, c };
        verts[i * 3 + 2] = (HermesVertex){ (float)cx + cosf(a1) * radius,      (float)cy + sinf(a1) * radius,      0, 0, c };
    }
    
    _GPU(painter)->draw_tris(_GPU(painter), verts, CIRCLE_SEGMENTS * 3);
    #undef CIRCLE_SEGMENTS
}
 
 
void 
_GPUSetClip(UIPainter *painter, UIRectangle clip)
{
    GPUPainterCtx *ctx = _GPUCtx(painter);
    DynamicArray_append((void **)&ctx->clip_stack, &painter->clip, 1);
    painter->clip = UIRectangleIntersection(painter->clip, clip);
    _GPU(painter)->set_clip(_GPU(painter), painter->clip);
}
 
 
void 
_GPURestoreClip(UIPainter *painter)
{
    GPUPainterCtx *ctx = _GPUCtx(painter);
    size_t len = DynamicArray_length(ctx->clip_stack);
    if (len > 0) {
        painter->clip = ctx->clip_stack[len - 1];
        DynamicArray_delete(ctx->clip_stack, len - 1, 1);
        _GPU(painter)->set_clip(_GPU(painter), painter->clip);
    }
}
 
 
void *
_GPUGetNative(UIPainter *painter)
{
    return _GPU(painter)->get_native(_GPU(painter));
}


/* Constructor / Destructor */
UIPainter 
GPUPainter_create(HermesGPUContext *gpu, int width, int height)
{
    if (!gpu) return (UIPainter){0};
    
    GPUPainterCtx *ctx = (GPUPainterCtx *)UI_MALLOC(sizeof(GPUPainterCtx));
    if (!ctx) return (UIPainter){0};

    ctx->gpu        = gpu;
    ctx->clip_stack = (UIRectangle *)DynamicArray(UIRectangle, 8);
    ctx->atlas      = _GlyphAtlasCreate();
 
    UIPainter p = {0};
    p.draw_block         = _GPUDrawBlock;
    p.draw_line          = _GPUDrawLine;
    p.draw_circle        = _GPUDrawCircle;
    p.draw_triangle      = _GPUDrawTriangle;
    p.draw_glyph         = _GPUDrawGlyph;
    p.draw_invert        = _GPUDrawInvert;
    p.draw_image         = _GPUDrawImage;
    p.set_clip           = _GPUSetClip;
    p.restore_clip       = _GPURestoreClip;
    p.get_native_context = _GPUGetNative;
    p.clip               = UI_RECT_4(0, width, 0, height);
    p.ctx                = ctx;
 
    gpu->begin(gpu);
 
    return p;
}
 
 
void 
GPUPainter_destroy(UIPainter *painter)
{
    GPUPainterCtx *ctx   = _GPUCtx(painter);
    if (!ctx) return;
    GlyphAtlas    *atlas = ctx->atlas;
 
    if (atlas) {
        if (atlas->texture)
            ctx->gpu->free_texture(ctx->gpu, atlas->texture);
        UI_FREE(atlas->bits);
        DynamicArray_free(atlas->entries);
        UI_FREE(atlas);
    }

    DynamicArray_free(ctx->clip_stack);
    
    ctx->gpu->present(ctx->gpu);
    
    UI_FREE(ctx);
}
