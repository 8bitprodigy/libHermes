#include "dynamicarray.h"
#include "font.h"
#include "ui_draw.h"
#include "ui.h"
#include "ui_checkbox.h"
#include "ui_string.h"
#include "utils.h"


#ifdef UI_SSE2
    #include <xmmintrin.h>
#endif


void 
_SWDrawBlock(UIPainter *painter, UIRectangle rectangle, uint32_t color)
{
    rectangle = UIRectangleIntersection(painter->clip, rectangle);

    if (!UI_RECT_VALID(rectangle)) {
        return;
    }

#ifdef UI_SSE2
    __m128i color4 = _mm_set_epi32(color, color, color, color);
#endif

    for (int line = rectangle.t; line < rectangle.b; line++) {
        uint32_t *bits  = ((SWPainterCtx *)painter->ctx)->bits + line * ((SWPainterCtx *)painter->ctx)->width + rectangle.l;
        int       count = UI_RECT_WIDTH(rectangle);

#ifdef UI_SSE2
        while (count >= 4) {
            _mm_storeu_si128((__m128i *)bits, color4);
            bits += 4;
            count -= 4;
        }
#endif

        while (count--) {
            *bits++ = color;
        }
    }

#ifdef UI_DEBUG
    painter->fillCount += UI_RECT_WIDTH(rectangle) * UI_RECT_HEIGHT(rectangle);
#endif
}


void 
_SWDrawImage(
    UIPainter   *painter, 
    UIRectangle  dst, 
    uint32_t    *src_bits, 
    int          src_width, 
    int          src_height
) 
{
    SWPainterCtx *ctx = (SWPainterCtx *)painter->ctx;
    UIRectangle clipped = UIRectangleIntersection(painter->clip, dst);
    if (!UI_RECT_VALID(clipped)) return;

    for (int y = clipped.t; y < clipped.b; y++) {
        uint32_t *dest_row = ctx->bits + y * ctx->width + clipped.l;
        uint32_t *src_row  = src_bits + (y - dst.t) * src_width + (clipped.l - dst.l);
        int       count    = UI_RECT_WIDTH(clipped);

#ifdef UI_SSE2
        __m128i *d = (__m128i *)dest_row;
        __m128i *s = (__m128i *)src_row;
        while (count >= 4) {
            _mm_storeu_si128(d++, _mm_loadu_si128(s++));
            count -= 4;
        }
        dest_row = (uint32_t *)d;
        src_row  = (uint32_t *)s;
#endif

        while (count--) {
            *dest_row++ = *src_row++;
        }
    }
}


void
_SWDrawLine(
    UIPainter *painter, 
    int        x0, 
    int        y0, 
    int        x1, 
    int        y1, 
    uint32_t   color
)
{
    // Apply the clip.

    UIRectangle c = painter->clip;
    if (!UI_RECT_VALID(c))
        return;
    int       dx = x1 - x0, dy = y1 - y0;
    const int p[4] = {-dx, dx, -dy, dy};
    const int q[4] = {x0 - c.l, c.r - 1 - x0, y0 - c.t, c.b - 1 - y0};
    float     t0 = 0.0f, t1 = 1.0f; // How far along the line the points end up.

    for (int i = 0; i < 4; i++) {
        if (!p[i] && q[i] < 0)
            return;
        float r = (float)q[i] / p[i];
        if (p[i] < 0 && r > t1)
            return;
        if (p[i] > 0 && r < t0)
            return;
        if (p[i] < 0 && r > t0)
            t0 = r;
        if (p[i] > 0 && r < t1)
            t1 = r;
    }

    x1 = x0 + t1 * dx, y1 = y0 + t1 * dy;
    x0 += t0 * dx, y0 += t0 * dy;

    // Calculate the delta X and delta Y.

    if (y1 < y0) {
        int t;
        t = x0, x0 = x1, x1 = t;
        t = y0, y0 = y1, y1 = t;
    }

    dx = x1 - x0, dy = y1 - y0;
    int dxs = dx < 0 ? -1 : 1;
    if (dx < 0)
        dx = -dx;

    // Draw the line using Bresenham's line algorithm.

    uint32_t *bits = ((SWPainterCtx *)painter->ctx)->bits + y0 * ((SWPainterCtx *)painter->ctx)->width + x0;

    if (dy * dy < dx * dx) {
        int m = 2 * dy - dx;

        for (int i = 0; i < dx; i++, bits += dxs) {
            *bits = color;
            if (m > 0)
                bits += ((SWPainterCtx *)painter->ctx)->width, m -= 2 * dx;
            m += 2 * dy;
        }
    } else {
        int m = 2 * dx - dy;

        for (int i = 0; i < dy; i++, bits += ((SWPainterCtx *)painter->ctx)->width) {
            *bits = color;
            if (m > 0)
                bits += dxs, m -= 2 * dy;
            m += 2 * dx;
        }
    }
}


void 
_SWDrawCircle(
    UIPainter *painter, 
    int        cx, 
    int        cy, 
    int        radius, 
    uint32_t   fillColor,
    uint32_t   outlineColor, 
    bool       hollow
)
{
    // TODO There's a hole missing at the bottom of the circle!
    // TODO This looks bad at small radii (< 20).

    float x = 0, y = -radius;
    float dx = radius, dy = 0;
    float step = 0.2f / radius;
    int   px = 0, py = cy + y;

    while (x >= 0) {
        x += dx * step;
        y += dy * step;
        dx += -x * step;
        dy += -y * step;

        int ix = x, iy = cy + y;

        while (py <= iy) {
            if (py >= painter->clip.t && py < painter->clip.b) {
                for (int s = 0; s <= ix || s <= px; s++) {
                    bool inOutline = ((s <= ix) != (s <= px)) || ((ix == px) && (s == ix));
                    if (hollow && !inOutline)
                        continue;
                    bool clip0 = cx + s >= painter->clip.l && cx + s < painter->clip.r;
                    bool clip1 = cx - s >= painter->clip.l && cx - s < painter->clip.r;
                    if (clip0)
                        ((SWPainterCtx *)painter->ctx)->bits[((SWPainterCtx *)painter->ctx)->width * py + cx + s] =
                            inOutline ? outlineColor : fillColor;
                    if (clip1)
                        ((SWPainterCtx *)painter->ctx)->bits[((SWPainterCtx *)painter->ctx)->width * py + cx - s] =
                            inOutline ? outlineColor : fillColor;
                }
            }

            px = ix, py++;
        }
    }
}


void 
_SWDrawTriangle(
    UIPainter *painter, 
    int        x0, 
    int        y0, 
    int        x1, 
    int        y1, 
    int        x2, 
    int        y2,
    uint32_t   color
)
{
    // Step 1: Sort the points by their y-coordinate.
    if (y1 < y0) {
        int xt = x0;
        x0 = x1, x1 = xt;
        int yt = y0;
        y0 = y1, y1 = yt;
    }
    if (y2 < y1) {
        int xt = x1;
        x1 = x2, x2 = xt;
        int yt = y1;
        y1 = y2, y2 = yt;
    }
    if (y1 < y0) {
        int xt = x0;
        x0 = x1, x1 = xt;
        int yt = y0;
        y0 = y1, y1 = yt;
    }
    if (y2 == y0)
        return;

    // Step 2: Clip the triangle.
    if (x0 < painter->clip.l && x1 < painter->clip.l && x2 < painter->clip.l)
        return;
    if (x0 >= painter->clip.r && x1 >= painter->clip.r && x2 >= painter->clip.r)
        return;
    if (y2 < painter->clip.t || y0 >= painter->clip.b)
        return;
    bool needsXClip = x0 < painter->clip.l + 1 || x0 >= painter->clip.r - 1 ||
                      x1 < painter->clip.l + 1 || x1 >= painter->clip.r - 1 ||
                      x2 < painter->clip.l + 1 || x2 >= painter->clip.r - 1;
    bool needsYClip = y0 < painter->clip.t + 1 || y2 >= painter->clip.b - 1;
#define _UI_DRAW_TRIANGLE_APPLY_CLIP(xo, yo)                                                       \
    if (needsYClip && (yi + yo < painter->clip.t || yi + yo >= painter->clip.b))                   \
        continue;                                                                                  \
    if (needsXClip && xf + xo < painter->clip.l)                                                   \
        xf = painter->clip.l - xo;                                                                 \
    if (needsXClip && xt + xo > painter->clip.r)                                                   \
        xt = painter->clip.r - xo;

    // Step 3: Split into 2 triangles with bases aligned with the x-axis.
    float xm0 = (float)((x2 - x0) * (y1 - y0)) / (y2 - y0), xm1 = x1 - x0;
    if (xm1 < xm0) {
        float xmt = xm0;
        xm0 = xm1, xm1 = xmt;
    }
    float xe0 = xm0 + x0 - x2, xe1 = xm1 + x0 - x2;
    int   ym = y1 - y0, ye = y2 - y1;
    float ymr = 1.0f / ym, yer = 1.0f / ye;

    // Step 4: Draw the top part.
    for (float y = 0; y < ym; y++) {
        int xf = xm0 * y * ymr, xt = xm1 * y * ymr, yi = (int)y;
        _UI_DRAW_TRIANGLE_APPLY_CLIP(x0, y0);
        uint32_t *b = &((SWPainterCtx *)painter->ctx)->bits[(yi + y0) * ((SWPainterCtx *)painter->ctx)->width + x0];
        for (int x = xf; x < xt; x++)
            b[x] = color;
    }

    // Step 5: Draw the bottom part.
    for (float y = 0; y < ye; y++) {
        int xf = xe0 * (ye - y) * yer, xt = xe1 * (ye - y) * yer, yi = (int)y;
        _UI_DRAW_TRIANGLE_APPLY_CLIP(x2, y1);
        uint32_t *b = &((SWPainterCtx *)painter->ctx)->bits[(yi + y1) * ((SWPainterCtx *)painter->ctx)->width + x2];
        for (int x = xf; x < xt; x++)
            b[x] = color;
    }
}

void 
_SWDrawInvert(UIPainter *painter, UIRectangle rectangle)
{
    rectangle = UIRectangleIntersection(painter->clip, rectangle);

    if (!UI_RECT_VALID(rectangle)) {
        return;
    }

    for (int line = rectangle.t; line < rectangle.b; line++) {
        uint32_t *bits  = ((SWPainterCtx *)painter->ctx)->bits + line * ((SWPainterCtx *)painter->ctx)->width + rectangle.l;
        int       count = UI_RECT_WIDTH(rectangle);

        while (count--) {
            uint32_t in = *bits;
            *bits       = in ^ 0xFFFFFF;
            bits++;
        }
    }
}


void
_SWSetClip(UIPainter *painter, UIRectangle clip)
{
    void **clip_stack_ptr = (void **)&painter->clip_stack;
    DynamicArray_append(clip_stack_ptr, &painter->clip, 1);
    painter->clip = UIRectangleIntersection(painter->clip, clip);
}


void
_SWRestoreClip(UIPainter *painter)
{
    size_t len = DynamicArray_length(painter->clip_stack);
    if (0 < len) {
        painter->clip = painter->clip_stack[len - 1];
        DynamicArray_delete(painter->clip_stack, len - 1, 1);
    }
}


UIPainter
SWPainter_create(uint32_t *bits, int width, int height)
{
    SWPainterCtx *ctx = (SWPainterCtx *)UI_MALLOC(sizeof(SWPainterCtx));;
    ctx->bits   = bits;
    ctx->width  = width;
    ctx->height = height;
    
    UIPainter p = {0};
    p.clip_stack    = DynamicArray(UIRectangle, 8);
    p.draw_block    = _SWDrawBlock;
    p.draw_image    = _SWDrawImage;
    p.draw_line     = _SWDrawLine;
    p.draw_circle   = _SWDrawCircle;
    p.draw_triangle = _SWDrawTriangle;
    p.draw_glyph    = _SWDrawGlyph;
    p.draw_invert   = _SWDrawInvert;
    p.set_clip      = _SWSetClip;
    p.restore_clip  = _SWRestoreClip;
    p.clip          = UI_RECT_4(0, width, 0, height);
    p.ctx           = ctx;

    return p;
}
