#ifndef HERMES_SDL3_H_
#define HERMES_SDL3_H_


#ifdef __cplusplus
extern "C" {
#endif


#ifdef UI_SDL3

	#include "ui_cursor.h"

	#include <SDL3/SDL.h>
	#include <stdbool.h>
	#include <string.h>


	/* Memory */
	#define UI_ASSERT           SDL_assert
	#define UI_CALLOC(x)        SDL_calloc(1, (x))
	#define UI_FREE             SDL_free
	#define UI_MALLOC           SDL_malloc
	#define UI_REALLOC          SDL_realloc
	#define UI_MEMMOVE(d, s, n) SDL_memmove((d), (s), (n))


	/* Clock */
	#define UI_CLOCK             SDL_GetTicks
	#define UI_CLOCKS_PER_SECOND 1000
	#define UI_CLOCK_T           uint64_t


/* Platform Global State */
typedef struct 
Hermes_Platform_SDL3 
{
    // Currently empty — SDL3 global state is managed through SDL itself.
    // Add fields here if needed (e.g. custom cursor handles).
    uint32_t user_event_type;
} 
Hermes_Platform_SDL3;


/* Per-window platform data */
typedef struct
Hermes_PlatformWindow
{
	SDL_Window  *sdl_window;
	SDL_Surface *sdl_surface;
}
Hermes_PlatformWindow;


#endif // UI_SDL3


#ifdef __cplusplus
}
#endif


#endif // HERMES_SDL3_H_
