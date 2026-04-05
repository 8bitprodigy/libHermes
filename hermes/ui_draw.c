#include "ui_draw.h"
#include "font.h"
#include "ui.h"
#include "ui_checkbox.h"
#include "ui_string.h"
#include "utils.h"


#ifdef UI_SSE2
    #include <xmmintrin.h>
#endif


uint32_t
shade_color(uint32_t color, uint8_t shade, float t)
{
    uint8_t
        r = (color >> 16) & 0xFF,
        g = (color >>  8) & 0xFF,
        b = (color >>  0) & 0xFF;

    r = (uint8_t)(r + (shade - r) * t);
    g = (uint8_t)(g + (shade - g) * t);
    b = (uint8_t)(b + (shade - b) * t);

    return (r << 16) | (g << 8) | (b << 0) | (color & 0xFF000000);
}


void 
UIDrawBlock(UIPainter *painter, UIRectangle rectangle, uint32_t color)
{
    painter->draw_block(painter, rectangle, color);
}


void
UIDrawLine(
    UIPainter *painter, 
    int        x0, 
    int        y0, 
    int        x1, 
    int        y1,
    uint32_t   color
)
{
    painter->draw_line(painter, x0, y0, x1, y1, color);
}


void 
UIDrawCircle(
    UIPainter *painter, 
    int        cx, 
    int        cy, 
    int        radius, 
    uint32_t   fill_color,
    uint32_t   outline_color, 
    bool       hollow
)
{
    painter->draw_circle(painter, cx, cy, radius, fill_color, outline_color, hollow);
}


void 
UIDrawTriangle(
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
    painter->draw_triangle(painter, x0, y0, x1, y1, x2, y2, color);
}

void 
UIDrawTriangleOutline(
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
    UIDrawLine(painter, x0, y0, x1, y1, color);
    UIDrawLine(painter, x1, y1, x2, y2, color);
    UIDrawLine(painter, x2, y2, x0, y0, color);
}


void 
UIDrawInvert(UIPainter *painter, UIRectangle rectangle)
{
    painter->draw_invert(painter, rectangle);
}


void
UISetClip(UIPainter *painter, UIRectangle rectangle)
{
    painter->set_clip(painter, rectangle);
}


void
UIRestoreClip(UIPainter *painter)
{
    painter->restore_clip(painter);
}


void UIDrawString(UIPainter *painter, UIRectangle r, const char *string, ptrdiff_t bytes,
                  uint32_t color, int align, UIStringSelection *selection)
{
    UIRectangle oldClip = painter->clip;
    painter->clip       = UIRectangleIntersection(r, oldClip);

    if (!UI_RECT_VALID(painter->clip)) {
        painter->clip = oldClip;
        return;
    }

    if (bytes == -1) {
        bytes = _UIStringLength(string);
    }

    int width  = UIMeasureStringWidth(string, bytes);
    int height = UIMeasureStringHeight();
    int x      = align == UI_ALIGN_CENTER  ? ((r.l + r.r - width) / 2)
                 : align == UI_ALIGN_RIGHT ? (r.r - width)
                                           : r.l;
    int y      = (r.t + r.b - height) / 2;
    int i = 0, j = 0;

    int selectFrom = -1, selectTo = -1;

    if (selection) {
        selectFrom = selection->carets[0];
        selectTo   = selection->carets[1];

        if (selectFrom > selectTo) {
            UI_SWAP(int, selectFrom, selectTo);
        }
    }

    while (j < bytes) {
        ptrdiff_t bytesConsumed = 1;
#ifdef UI_UNICODE
        int c = Utf8GetCodePoint(string, bytes - j, &bytesConsumed);
        UI_ASSERT(bytesConsumed > 0);
        string += bytesConsumed;
#else
        char c = *string++;
#endif
        uint32_t colorText = color;

        if (i >= selectFrom && i < selectTo) {
#ifdef UI_FREETYPE
    if (ui.activeFont->isFreeType) UIEnsureGlyphRendered(ui.activeFont, c);
#endif
            int w = GLYPH_ADVANCE(c);
            if (c == '\t') {
                int ii = i;
                while (++ii & 3)
                    w += GLYPH_ADVANCE(c);
            }
            UIDrawBlock(painter, UI_RECT_4(x, x + w, y, y + height), selection->colorBackground);
            colorText = selection->colorText;
        }

        if (c != '\t') {
            UIDrawGlyph(painter, x, y, c, colorText);
        }

        if (selection && selection->carets[0] == i) {
            UIDrawInvert(painter, UI_RECT_4(x, x + 1, y, y + height));
        }
        
        x += GLYPH_ADVANCE(c), i++;
        
        if (c == '\t') {
            while (i & 3)
                x += GLYPH_ADVANCE(c);
        }

        j += bytesConsumed;
    }

    if (selection && selection->carets[0] == i) {
        UIDrawInvert(painter, UI_RECT_4(x, x + 1, y, y + height));
    }

    painter->clip = oldClip;
}


void 
UIDrawBorder(
    UIPainter *painter, 
    UIRectangle r, 
    uint32_t main_color, 
    UIRectangle border_size, 
    BorderType border_type
)
{
    uint32_t
        highlight = shade_color(main_color, 255, 0.3f),
        shadow    = shade_color(main_color,   0, 0.5f);

    if (border_type) {
        uint32_t swap = highlight;
        highlight = shadow;
        shadow    = swap;
    }
    UIDrawBlock(
            painter, 
            UI_RECT_4(r.l, r.r, r.t, r.t + border_size.t), 
            highlight
        );
    UIDrawBlock(
            painter, 
            UI_RECT_4(r.l, r.l + border_size.l, r.t + border_size.t, r.b - border_size.b),
            highlight
        );
    UIDrawBlock(
            painter, 
            UI_RECT_4(r.r - border_size.r, r.r, r.t + border_size.t, r.b - border_size.b),
            shadow
        );
    UIDrawBlock(
            painter, 
            UI_RECT_4(r.l, r.r, r.b - border_size.b, r.b), 
            shadow
        );
}


void 
UIDrawRectangle(
    UIPainter   *painter, 
    UIRectangle  r, 
    uint32_t     main_color, 
    uint32_t     border_color,
    UIRectangle  border_size,
    BorderType   border_type
)
{
    UIDrawBorder(painter, r, main_color, border_size, border_type);
    UIDrawBlock(
        painter,
        UI_RECT_4(
                r.l + border_size.l, 
                r.r - border_size.r, 
                r.t + border_size.t, 
                r.b - border_size.b
            ),
        main_color);
}


void 
UIDrawControlDefault(
    UIPainter   *painter, 
    UIRectangle  bounds, 
    uint32_t     mode, 
    const char  *label,
    ptrdiff_t    labelBytes, 
    double       position, 
    float        scale
)
{
    bool     
        checked       = mode & UI_DRAW_CONTROL_STATE_CHECKED,
        disabled      = mode & UI_DRAW_CONTROL_STATE_DISABLED,
        focused       = mode & UI_DRAW_CONTROL_STATE_FOCUSED,
        hovered       = mode & UI_DRAW_CONTROL_STATE_HOVERED,
        indeterminate = mode & UI_DRAW_CONTROL_STATE_INDETERMINATE,
        pressed       = mode & UI_DRAW_CONTROL_STATE_PRESSED,
        selected      = mode & UI_DRAW_CONTROL_STATE_SELECTED;
    uint32_t which    = mode & UI_DRAW_CONTROL_TYPE_MASK;

    uint32_t buttonColor = disabled ? 
            ui.theme.buttonDisabled
            : (pressed && hovered) ? 
                    ui.theme.buttonPressed
                    : (pressed || hovered || checked) ? 
                            ui.theme.buttonHovered
                            : focused ? 
                                    ui.theme.selected
                                    : ui.theme.buttonNormal;
    
    uint32_t buttonTextColor = disabled ? 
            ui.theme.textDisabled
            : buttonColor == ui.theme.selected ? 
                ui.theme.textSelected
                : ui.theme.text;
    
    switch (which) {
    case UI_DRAW_CONTROL_CHECKBOX: {
            uint32_t
                color     = buttonColor, 
                textColor = buttonTextColor;
            int midY = (bounds.t + bounds.b) / 2;
            UIRectangle boxBounds =UI_RECT_4(
                    bounds.l, 
                    bounds.l + UI_SIZE_CHECKBOX_BOX, 
                    midY - UI_SIZE_CHECKBOX_BOX / 2,
                    midY + UI_SIZE_CHECKBOX_BOX / 2
                );
            UIDrawRectangle(
                    painter,
                    boxBounds, 
                    color, 
                    ui.theme.border, 
                    UI_RECT_1(1), 
                    (checked || indeterminate || pressed) ? 
                        BORDER_LOWERED 
                        : BORDER_RAISED
                );
            UIDrawString(painter, UIRectangleAdd(boxBounds, UI_RECT_4(1, 0, 0, 0)),
                        checked         ? "\u2713"
                        : indeterminate ? "-"
                                        : " ",
                        -1, textColor, UI_ALIGN_CENTER, NULL);
            UIDrawString(
                painter,
                UIRectangleAdd(bounds, UI_RECT_4(UI_SIZE_CHECKBOX_BOX + UI_SIZE_CHECKBOX_GAP, 0, 0, 0)),
                label, labelBytes, disabled ? ui.theme.textDisabled : ui.theme.text, UI_ALIGN_LEFT,
                NULL);
            break;
        }
    case UI_DRAW_CONTROL_RADIOBUTTON: {
            uint32_t
                color     = buttonColor, 
                textColor = buttonTextColor,
                
                highlight = shade_color(color, 255, 0.3f),
                ambient   = shade_color(color, 127, 0.4f),
                shadow    = shade_color(color,   0, 0.5f);
                
            if (checked || pressed) {
                uint32_t swap = highlight;
                highlight = shadow;
                shadow    = swap;
            }
            
            int midY = (bounds.t + bounds.b) / 2;
            UIRectangle  boxBounds = UI_RECT_4(
                    bounds.l, 
                    bounds.l + UI_SIZE_CHECKBOX_BOX, 
                    midY - UI_SIZE_CHECKBOX_BOX / 2,
                    midY + UI_SIZE_CHECKBOX_BOX / 2
                );
            int 
                rad_mid_X = (boxBounds.l + boxBounds.r) / 2,
                rad_mid_Y = (boxBounds.t + boxBounds.b) / 2;

            /* Background of radio control */
            UIDrawTriangle(
                    painter, 
                    boxBounds.l,     rad_mid_Y, 
                    rad_mid_X, boxBounds.t, 
                    boxBounds.r, rad_mid_Y, 
                    color
                );
            UIDrawTriangle(
                    painter, 
                    boxBounds.r,     rad_mid_Y, 
                    rad_mid_X, boxBounds.b, 
                    boxBounds.l, rad_mid_Y, 
                    color
                );

            /* Border of radio control */
            UIDrawLine(
                    painter, 
                    boxBounds.l, rad_mid_Y, 
                    rad_mid_X,   boxBounds.t,
                    highlight
                );
            UIDrawLine(
                    painter, 
                    rad_mid_X   + 1,   boxBounds.t + 1, 
                    boxBounds.r + 1, rad_mid_Y     + 1, 
                    ambient
                );
            UIDrawLine(
                    painter, 
                    boxBounds.r - 1, rad_mid_Y     + 1, 
                    rad_mid_X   - 1,   boxBounds.b + 1, 
                    shadow
                );
            UIDrawLine(
                    painter, 
                    rad_mid_X, boxBounds.b, 
                    boxBounds.l, rad_mid_Y, 
                    ambient
                );
            
            UIDrawString(painter, UIRectangleAdd(boxBounds, UI_RECT_4(1, 0, 2, 0)),
                        checked ? 
                            "\u2022"
                            : " ",
                        -1, textColor, UI_ALIGN_CENTER, NULL);
            UIDrawString(
                painter,
                UIRectangleAdd(bounds, UI_RECT_4(UI_SIZE_CHECKBOX_BOX + UI_SIZE_CHECKBOX_GAP, 0, 0, 0)),
                label, labelBytes, disabled ? ui.theme.textDisabled : ui.theme.text, UI_ALIGN_LEFT,
                NULL);
            break;
        }
    case UI_DRAW_CONTROL_MENU_ITEM:    /* FALLTHROUGH */
    case UI_DRAW_CONTROL_DROP_DOWN:    /* FALLTHROUGH */
    case UI_DRAW_CONTROL_PUSH_BUTTON: {
            uint32_t color = buttonColor, textColor = buttonTextColor;
            int      borderSize = which == UI_DRAW_CONTROL_MENU_ITEM ? 0 : scale;
            UIDrawRectangle(painter, bounds, color, ui.theme.border, UI_RECT_1(borderSize), pressed ? BORDER_LOWERED : BORDER_RAISED);

            if (checked && !focused) {
                UIDrawBlock(
                    painter,
                    UIRectangleAdd(bounds, UI_RECT_1I((int)(UI_SIZE_BUTTON_CHECKED_AREA * scale))),
                    ui.theme.buttonPressed);
            }

            UIRectangle innerBounds =
                UIRectangleAdd(bounds, UI_RECT_2I((int)(UI_SIZE_MENU_ITEM_MARGIN * scale), 0));

            if (which == UI_DRAW_CONTROL_MENU_ITEM) {
                if (labelBytes == -1) {
                    labelBytes = _UIStringLength(label);
                }

                int tab = 0;
                for (; tab < labelBytes && label[tab] != '\t'; tab++)
                    ;

                UIDrawString(painter, innerBounds, label, tab, textColor, UI_ALIGN_LEFT, NULL);

                if (labelBytes > tab) {
                    UIDrawString(painter, innerBounds, label + tab + 1, labelBytes - tab - 1, textColor,
                                UI_ALIGN_RIGHT, NULL);
                }
            } else if (which == UI_DRAW_CONTROL_DROP_DOWN) {
                UIDrawString(painter, innerBounds, label, labelBytes, textColor, UI_ALIGN_LEFT, NULL);
                UIDrawString(painter, innerBounds, "\x19", 1, textColor, UI_ALIGN_RIGHT, NULL);
            } else {
                UIDrawString(painter, bounds, label, labelBytes, textColor, UI_ALIGN_CENTER, NULL);
            }
            break;
        }
    case UI_DRAW_CONTROL_LABEL:
        UIDrawString(painter, bounds, label, labelBytes, ui.theme.text, UI_ALIGN_LEFT, NULL);
        break;
    case UI_DRAW_CONTROL_SPLITTER: {
            UIRectangle borders =
                (mode & UI_DRAW_CONTROL_STATE_VERTICAL) ? UI_RECT_2(0, 1) : UI_RECT_2(1, 0);
            UIDrawRectangle(painter, bounds, ui.theme.buttonNormal, ui.theme.border, borders, BORDER_RAISED);
            break;
        }
    case UI_DRAW_CONTROL_SCROLL_TRACK: 
        if (disabled)
            UIDrawBlock(painter, bounds, ui.theme.panel1);
        
        break;
    case UI_DRAW_CONTROL_SCROLL_DOWN:    /* FALLTHROUGH */
    case UI_DRAW_CONTROL_SCROLL_UP: {
            bool     isDown = which == UI_DRAW_CONTROL_SCROLL_DOWN;
            uint32_t color  = pressed ? 
                    ui.theme.buttonPressed
                    : hovered ? 
                            ui.theme.buttonHovered
                            : ui.theme.panel2;
            UIDrawRectangle(
                    painter, 
                    bounds, 
                    color, 
                    ui.theme.border, 
                    UI_RECT_1(0), 
                    BORDER_RAISED
                );

            if (mode & UI_DRAW_CONTROL_STATE_VERTICAL) {
                UIDrawGlyph(
                        painter, 
                        (bounds.l + bounds.r - ui.activeFont->glyphWidth) 
                            / 2 + 1,
                        isDown ? 
                            (bounds.b - ui.activeFont->glyphHeight - 2 * scale)
                            : (bounds.t + 2 * scale),
                        isDown ? 25 : 24, 
                        ui.theme.text
                    );
            } else {
                UIDrawGlyph(
                        painter,
                        isDown ? 
                            (bounds.r - ui.activeFont->glyphWidth - 2 * scale)
                            : (bounds.l + 2 * scale),
                        (bounds.t + bounds.b - ui.activeFont->glyphHeight) / 2, 
                        isDown ? 26 : 27,
                        ui.theme.text
                    );
            }
            break;
        }
    case UI_DRAW_CONTROL_SCROLL_THUMB: {
            uint32_t color = pressed ? 
                    ui.theme.buttonPressed
                    : hovered ? 
                            ui.theme.buttonHovered
                            : ui.theme.buttonNormal;
            UIDrawRectangle(
                    painter, 
                    bounds, 
                    color, 
                    ui.theme.border, 
                    UI_RECT_1(1), 
                    BORDER_RAISED
                );
            break;
        }
    case UI_DRAW_CONTROL_GAUGE:
        UIDrawRectangle(painter, bounds, ui.theme.panel2, ui.theme.border, UI_RECT_1(1), BORDER_LOWERED);
        UIRectangle filled = UIRectangleAdd(bounds, UI_RECT_1I(1));
        if (mode & UI_DRAW_CONTROL_STATE_VERTICAL) {
            filled.t = filled.b - UI_RECT_HEIGHT(filled) * position;
        } else {
            filled.r = filled.l + UI_RECT_WIDTH(filled) * position;
        }
        UIDrawRectangle(painter, filled, ui.theme.selected, 0, UI_RECT_1(1), BORDER_RAISED);
        break;
    case UI_DRAW_CONTROL_SLIDER: {
            bool vertical     = mode & UI_DRAW_CONTROL_STATE_VERTICAL;
            int         
                centerX       = (bounds.r + bounds.l) / 2,
                centerY       = (bounds.t + bounds.b) / 2,
                center        = vertical ? centerX : centerY,
                trackSize     = UI_SIZE_SLIDER_TRACK * scale,
                thumbSize     = UI_SIZE_SLIDER_THUMB * scale,
                thumbPosition = vertical ? 
                        (UI_RECT_HEIGHT(bounds) - thumbSize) * position
                        : (UI_RECT_WIDTH(bounds) - thumbSize) * position;
            UIRectangle track = vertical ? 
                    UI_RECT_4(
                            center - (trackSize + 1) / 2,
                            center + trackSize / 2, 
                            bounds.t, 
                            bounds.b
                        )
                    : UI_RECT_4(
                            bounds.l, 
                            bounds.r, 
                            center - (trackSize + 1) / 2,
                            center + trackSize / 2
                        );
            UIDrawRectangle(
                    painter, 
                    track, 
                    disabled ? ui.theme.panel1 : ui.theme.panel2,
                    ui.theme.border, 
                    UI_RECT_1(1), 
                    BORDER_LOWERED
                );
            uint32_t color = disabled  ? 
                    ui.theme.buttonDisabled
                    : pressed ? 
                            ui.theme.buttonPressed
                            : hovered ? 
                                    ui.theme.buttonHovered
                                    : ui.theme.buttonNormal;
            UIRectangle thumb = vertical ? 
                    UI_RECT_4(
                            center - (thumbSize + 1) / 2, 
                            center + thumbSize / 2,
                            bounds.b - thumbPosition - thumbSize, 
                            bounds.b - thumbPosition
                        )
                    : UI_RECT_4(
                            bounds.l + thumbPosition,
                            bounds.l + thumbPosition + thumbSize,
                            center - (thumbSize + 1) / 2, 
                            center + thumbSize / 2
                        );
            UIDrawRectangle(
                    painter, 
                    thumb, 
                    color, 
                    ui.
                    theme.border, 
                    UI_RECT_1(1), 
                    BORDER_RAISED
                );
            break;
        }
    case UI_DRAW_CONTROL_TEXTBOX:
        UIDrawRectangle(
                painter, 
                bounds,
                disabled  ? 
                        ui.theme.buttonDisabled
                        : focused ?
                                ui.theme.textboxFocused
                                : ui.theme.textboxNormal,
                ui.theme.border, 
                UI_RECT_1(1), 
                BORDER_LOWERED
            );
        break;
    case UI_DRAW_CONTROL_MODAL_POPUP:
        UIRectangle bounds2 = UIRectangleAdd(bounds, UI_RECT_1I(-1));
        UIDrawBorder(painter, bounds2, ui.theme.border, UI_RECT_1(1), BORDER_RAISED);
        UIDrawBorder(painter, UIRectangleAdd(bounds2, UI_RECT_1(1)), ui.theme.border, UI_RECT_1(1), BORDER_RAISED);
        break;
    case UI_DRAW_CONTROL_MENU:
        UIDrawBlock(painter, bounds, ui.theme.border);
        break;
    case UI_DRAW_CONTROL_TABLE_ROW:
        if (selected)
            UIDrawBlock(painter, bounds, ui.theme.selected);
        else if (hovered)
            UIDrawBlock(painter, bounds, ui.theme.buttonHovered);
        break;
    case UI_DRAW_CONTROL_TABLE_CELL:
        uint32_t textColor = selected ? ui.theme.textSelected : ui.theme.text;
        UIDrawString(painter, bounds, label, labelBytes, textColor, UI_ALIGN_LEFT, NULL);
        break;
    case UI_DRAW_CONTROL_TABLE_BACKGROUND:
        UIDrawRectangle(
                painter, 
                bounds, 
                ui.theme.panel2, 
                ui.theme.border, 
                UI_RECT_1(1), 
                BORDER_LOWERED
            );
        UIDrawRectangle(
                painter,
                UI_RECT_4(bounds.l, bounds.r, bounds.t, bounds.t + (int)(UI_SIZE_TABLE_HEADER * scale)),
                ui.theme.panel1, 
                ui.theme.border, 
                UI_RECT_4(0, 0, 0, 1), 
                BORDER_RAISED
            );
        break;
    case UI_DRAW_CONTROL_TABLE_HEADER:
        UIDrawString(painter, bounds, label, labelBytes, ui.theme.text, UI_ALIGN_LEFT, NULL);
        if (selected)
            UIDrawInvert(painter, bounds);
        break;
    case UI_DRAW_CONTROL_MDI_CHILD: {
            UI_MDI_CHILD_CALCULATE_LAYOUT(bounds, scale);
            UIRectangle borders = UI_RECT_4(borderSize, borderSize, titleSize, borderSize);
            UIDrawBorder(painter, bounds, ui.theme.buttonNormal, borders, BORDER_RAISED);
            UIDrawBorder(painter, bounds, ui.theme.border, UI_RECT_1((int)scale), BORDER_RAISED);
            UIDrawBorder(
                    painter, 
                    UIRectangleAdd(content, UI_RECT_1I(-1)), 
                    ui.theme.border,
                    UI_RECT_1((int)scale),
                    BORDER_RAISED
                );
            UIDrawString(painter, title, label, labelBytes, ui.theme.text, UI_ALIGN_LEFT, NULL);
            break;
        }
    case UI_DRAW_CONTROL_TAB: {
            uint32_t    color = selected ? ui.theme.buttonNormal : ui.theme.buttonPressed;
            UIRectangle t     = bounds;
            t.r--;
            if (selected)
                t.t--;
            else
                t.b--, t.t++;
            
            UIDrawRectangle(painter, t, color, ui.theme.border, UI_RECT_4(1, 1, 1, 0), BORDER_RAISED);
            UIDrawString(painter, bounds, label, labelBytes, ui.theme.text, UI_ALIGN_CENTER, NULL);
            break;
        }
    case UI_DRAW_CONTROL_TAB_BAND:
        UIDrawRectangle(painter, bounds, ui.theme.panel2, ui.theme.border, UI_RECT_1(1), BORDER_LOWERED);
        break;
    }
}
