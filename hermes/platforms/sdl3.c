#include "sdl3.h"
#include "ui.h"
#include "ui_animation.h"
#include "ui_event.h"
#include "ui_key.h"
#include "ui_menu.h"
#include "ui_window.h"
#include "utils.h"


#ifdef UI_SDL3


// Key code constants 

const int UI_KEYCODE_A         = SDLK_A;
const int UI_KEYCODE_BACKSPACE = SDLK_BACKSPACE;
const int UI_KEYCODE_DELETE    = SDLK_DELETE;
const int UI_KEYCODE_DOWN      = SDLK_DOWN;
const int UI_KEYCODE_END       = SDLK_END;
const int UI_KEYCODE_ENTER     = SDLK_RETURN;
const int UI_KEYCODE_ESCAPE    = SDLK_ESCAPE;
const int UI_KEYCODE_F1        = SDLK_F1;
const int UI_KEYCODE_HOME      = SDLK_HOME;
const int UI_KEYCODE_LEFT      = SDLK_LEFT;
const int UI_KEYCODE_RIGHT     = SDLK_RIGHT;
const int UI_KEYCODE_SPACE     = SDLK_SPACE;
const int UI_KEYCODE_TAB       = SDLK_TAB;
const int UI_KEYCODE_UP        = SDLK_UP;
const int UI_KEYCODE_INSERT    = SDLK_INSERT;
const int UI_KEYCODE_0         = SDLK_0;
const int UI_KEYCODE_BACKTICK  = SDLK_GRAVE;
const int UI_KEYCODE_PAGE_DOWN = SDLK_PAGEDOWN;
const int UI_KEYCODE_PAGE_UP   = SDLK_PAGEUP;


// Helpers 

static UIWindow *
_SDLFindWindow(SDL_WindowID id)
{
    UIWindow *w = ui.windows;
    while (w) {
        if (SDL_GetWindowID(w->window.sdl_window) == id)
            return w;
        w = w->next;
    }
    return NULL;
}


static void 
_SDLResizeFramebuffer(UIWindow *window, int width, int height)
{
    UI_FREE(window->bits);
    window->bits   = (uint32_t *)UI_MALLOC(width * height * 4);
    window->width  = width;
    window->height = height;

    if (window->window.sdl_surface) {
        SDL_DestroySurface(window->window.sdl_surface);
    }

    window->window.sdl_surface = SDL_CreateSurfaceFrom(
        width, height, SDL_PIXELFORMAT_XRGB8888,
        window->bits, width * 4
    );
}


// Platform functions 

const char *
Hermes_GetBackendName(void) 
{
    return "SDL3";
}

Hermes_Platform *
Hermes_PlatformInit(void)
{
    Hermes_Platform_SDL3 *platform = calloc(1, sizeof(*platform));
    if (!platform) return NULL;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        free(platform);
        return NULL;
    }

    platform->user_event_type = SDL_RegisterEvents(1);
    return (Hermes_Platform *)platform;
}


void 
Hermes_Platform_CreateWindow(
    UIWindow   *window, 
    uint32_t    flags, 
    const char *cTitle,
    int         _width, 
    int         _height
)
{
    _UIMenusClose();

    int width  = (flags & UI_WINDOW_MENU) ? 1 : _width  ? _width  : 800;
    int height = (flags & UI_WINDOW_MENU) ? 1 : _height ? _height : 600;

    SDL_WindowFlags sdl_flags = SDL_WINDOW_RESIZABLE;

    if (flags & UI_WINDOW_MENU) {
        sdl_flags |= SDL_WINDOW_BORDERLESS | SDL_WINDOW_POPUP_MENU;
    }

    if (flags & UI_WINDOW_MAXIMIZE) {
        sdl_flags |= SDL_WINDOW_MAXIMIZED;
    }

    window->window.sdl_window = SDL_CreateWindow(
        cTitle ? cTitle : "", width, height, sdl_flags
    );

    if (!window->window.sdl_window) return;

    if (flags & UI_WINDOW_CENTER_IN_OWNER && window->owner) {
        int ox, oy;
        Hermes_Platform_get_screen_pos(&window->owner->window, &ox, &oy);
        SDL_SetWindowPosition(
            window->window.sdl_window,
            ox + (window->owner->width  / 2) - (width  / 2),
            oy + (window->owner->height / 2) - (height / 2)
        );
    }

    window->window.sdl_surface = NULL;
    _SDLResizeFramebuffer(window, width, height);
    SDL_StartTextInput(window->window.sdl_window);
    
    UIElementMove(&window->e, UI_RECT_2S(width, height), true);
    UIElementRepaint(&window->e, NULL);
}


void 
Hermes_Platform_DestroyWindow(Hermes_PlatformWindow *window)
{
    if (window->sdl_surface) {
        SDL_DestroySurface(window->sdl_surface);
        window->sdl_surface = NULL;
    }
    if (window->sdl_window) {
        SDL_DestroyWindow(window->sdl_window);
        window->sdl_window = NULL;
    }
}


void 
Hermes_Platform_get_screen_pos(Hermes_PlatformWindow *pwindow, int *_x, int *_y)
{
    SDL_GetWindowPosition(pwindow->sdl_window, _x, _y);
}


void Hermes_Platform_render(UIWindow *window, UIPainter *painter)
{
    SDL_Surface *screen = SDL_GetWindowSurface(window->window.sdl_window);
    if (!screen) return;
    
    // Recreate source surface if size changed
    if (!window->window.sdl_surface ||
        window->window.sdl_surface->w != window->width ||
        window->window.sdl_surface->h != window->height) {
        if (window->window.sdl_surface)
            SDL_DestroySurface(window->window.sdl_surface);
        window->window.sdl_surface = SDL_CreateSurfaceFrom(
            window->width, window->height,
            SDL_PIXELFORMAT_XRGB8888,
            window->bits,
            window->width * 4
        );
    }
    
    SDL_BlitSurface(window->window.sdl_surface, NULL, screen, NULL);
    SDL_UpdateWindowSurface(window->window.sdl_window);
}


// Event processing 

static void 
_SDLProcessEvent(SDL_Event *ev)
{
    UIWindow *window = NULL;
    if (ev->type >= SDL_EVENT_WINDOW_FIRST && ev->type <= SDL_EVENT_WINDOW_LAST)
        window = _SDLFindWindow(ev->window.windowID);
    else if (ev->type == SDL_EVENT_MOUSE_MOTION)
        window = _SDLFindWindow(ev->motion.windowID);
    else if (ev->type == SDL_EVENT_MOUSE_BUTTON_DOWN || ev->type == SDL_EVENT_MOUSE_BUTTON_UP)
        window = _SDLFindWindow(ev->button.windowID);
    else if (ev->type == SDL_EVENT_MOUSE_WHEEL)
        window = _SDLFindWindow(ev->wheel.windowID);
    else if (ev->type == SDL_EVENT_KEY_DOWN || ev->type == SDL_EVENT_KEY_UP)
        window = _SDLFindWindow(ev->key.windowID);
    else if (ev->type == SDL_EVENT_TEXT_INPUT)
        window = _SDLFindWindow(ev->text.windowID);

    switch (ev->type) {
    case SDL_EVENT_QUIT:
        ui.quit = true;
        break;

    case SDL_EVENT_WINDOW_RESIZED: {
            if (!window) break;
            int w = ev->window.data1, h = ev->window.data2;
            _SDLResizeFramebuffer(window, w, h);
            UIElementMove(&window->e, UI_RECT_2S(w, h), true);
            UIElementRepaint(&window->e, NULL);
            break;
        }

    case SDL_EVENT_WINDOW_EXPOSED:
        if (window) UIElementRepaint(&window->e, NULL);
        break;

    case SDL_EVENT_WINDOW_FOCUS_GAINED:
        if (window) _UIWindowInputEvent(window, UI_MSG_WINDOW_ACTIVATE, 0, NULL);
        break;

    case SDL_EVENT_WINDOW_FOCUS_LOST:
        //if (window) _UIWindowInputEvent(window, UI_MSG_WINDOW_DEACTIVATE, 0, NULL);
        break;

    case SDL_EVENT_MOUSE_MOTION: {
            if (!window) break;
            window->cursorX = ev->motion.x;
            window->cursorY = ev->motion.y;
            _UIWindowInputEvent(window, UI_MSG_MOUSE_MOVE, 0, NULL);
            break;
        }

    case SDL_EVENT_MOUSE_BUTTON_DOWN: /* FALLTHROUGH */
    case SDL_EVENT_MOUSE_BUTTON_UP: {
            if (!window) break;
            window->cursorX = ev->button.x;
            window->cursorY = ev->button.y;
            UIMessage msg;
            bool down = ev->type == SDL_EVENT_MOUSE_BUTTON_DOWN;
            switch (ev->button.button) {
                case SDL_BUTTON_LEFT:   msg = down ? UI_MSG_LEFT_DOWN   : UI_MSG_LEFT_UP;   break;
                case SDL_BUTTON_MIDDLE: msg = down ? UI_MSG_MIDDLE_DOWN : UI_MSG_MIDDLE_UP; break;
                case SDL_BUTTON_RIGHT:  msg = down ? UI_MSG_RIGHT_DOWN  : UI_MSG_RIGHT_UP;  break;
                default: return;
            }
            _UIWindowInputEvent(window, msg, 0, NULL);
            break;
        }

    case SDL_EVENT_MOUSE_WHEEL: {
            if (!window) break;
            _UIWindowInputEvent(window, UI_MSG_MOUSE_WHEEL, (int)(-ev->wheel.y * 72), NULL);
            break;
        }

    case SDL_EVENT_KEY_DOWN: /* FALLTHROUGH */
    case SDL_EVENT_KEY_UP: {
            if (!window) break;
            SDL_Keymod mod = ev->key.mod;
            window->ctrl  = (mod & SDL_KMOD_CTRL)  != 0;
            window->shift = (mod & SDL_KMOD_SHIFT) != 0;
            window->alt   = (mod & SDL_KMOD_ALT)   != 0;

            if (ev->type == SDL_EVENT_KEY_DOWN) {
                UIKeyTyped m = {0};
                m.code = ev->key.key;
                _UIWindowInputEvent(window, UI_MSG_KEY_TYPED, 0, &m);
            }
            break;
        }

    case SDL_EVENT_TEXT_INPUT: {
            if (!window) break;
            UIKeyTyped m = {0};
            m.text      = (char*)ev->text.text;
            m.textBytes = strlen(ev->text.text);
            _UIWindowInputEvent(window, UI_MSG_KEY_TYPED, 0, &m);
            break;
        }

    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        if (window) _UIWindowInputEvent(window, UI_MSG_WINDOW_CLOSE, 0, NULL);
        break;
    }
}


bool _UIMessageLoopSingle(int *result)
{
    SDL_Event ev;

    // Paint any pending updates before blocking
    Hermes_UpdateUI();
    
    if (ui.animatingCount) {
        if (SDL_PollEvent(&ev)) {
            _SDLProcessEvent(&ev);
        } else {
            _UIProcessAnimations();
        }
        return !ui.quit;
    }

    // Block until an event arrives
    if (!SDL_WaitEvent(&ev)) return !ui.quit;
    _SDLProcessEvent(&ev);

    // Drain remaining events
    while (SDL_PollEvent(&ev)) {
        _SDLProcessEvent(&ev);
    }

    Hermes_UpdateUI();
    return !ui.quit;
}


void UIMenuShow(UIMenu *menu)
{
    // Position the menu window at the cursor of its parent window
    UIWindow *parent = menu->e.window;
    int x, y;
    Hermes_Platform_get_screen_pos(&parent->window, &x, &y);
    SDL_SetWindowPosition(
            menu->e.window->window.sdl_window,
            x + parent->cursorX,
            y + parent->cursorY
        );
    SDL_ShowWindow(menu->e.window->window.sdl_window);
}


void
_UIClipboardWriteText(UIWindow *window, char *text)
{
    SDL_SetClipboardText(text);
}

char *
_UIClipboardReadTextStart(UIWindow *window, size_t *bytes)
{
    char *text = SDL_GetClipboardText();
    if (!text) { *bytes = 0; return NULL; }
    *bytes = SDL_strlen(text);
    return text;
}

void
_UIClipboardReadTextEnd(UIWindow *window, char *text)
{
    SDL_free(text);
}


void 
_UIWindowSetCursor(UIWindow *window, int cursor)
{
    static const SDL_SystemCursor map[] = {
            SDL_SYSTEM_CURSOR_DEFAULT,   // ARROW
            SDL_SYSTEM_CURSOR_TEXT,      // TEXT
            SDL_SYSTEM_CURSOR_NS_RESIZE, // SPLIT_V
            SDL_SYSTEM_CURSOR_EW_RESIZE, // SPLIT_H
            SDL_SYSTEM_CURSOR_DEFAULT,   // FLIPPED_ARROW
            SDL_SYSTEM_CURSOR_CROSSHAIR, // CROSS_HAIR
            SDL_SYSTEM_CURSOR_POINTER,   // HAND
            SDL_SYSTEM_CURSOR_N_RESIZE,  // RESIZE_UP
            SDL_SYSTEM_CURSOR_W_RESIZE,  // RESIZE_LEFT
            SDL_SYSTEM_CURSOR_NE_RESIZE, // RESIZE_UP_RIGHT
            SDL_SYSTEM_CURSOR_NW_RESIZE, // RESIZE_UP_LEFT
            SDL_SYSTEM_CURSOR_S_RESIZE,  // RESIZE_DOWN
            SDL_SYSTEM_CURSOR_E_RESIZE,  // RESIZE_RIGHT
            SDL_SYSTEM_CURSOR_SE_RESIZE, // RESIZE_DOWN_RIGHT
            SDL_SYSTEM_CURSOR_SW_RESIZE, // RESIZE_DOWN_LEFT
        };
    SDL_SetCursor(SDL_CreateSystemCursor(map[cursor]));
}


void UIWindowPostMessage(UIWindow *window, UIMessage message, void *dp)
{
    SDL_Event ev = {0};
    Hermes_Platform_SDL3 *platform = (Hermes_Platform_SDL3*)ui.platform;
    ev.type           = platform->user_event_type;
    ev.user.windowID  = SDL_GetWindowID(window->window.sdl_window);
    ev.user.code      = (int)message;
    ev.user.data1     = dp;
    SDL_PushEvent(&ev);
}


#endif // UI_SDL3
