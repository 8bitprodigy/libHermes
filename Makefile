CC = gcc
 
# X11 backend (default)
CFLAGS_X11 = -g3 -O0 -I. -I./include -DUI_LINUX
LIBS_X11   = -lX11 -lm
 
# Optional: comment out these two lines to disable FreeType
CFLAGS_X11 += -DUI_FREETYPE -DUI_UNICODE $(shell pkg-config --cflags freetype2)
LIBS_X11   += $(shell pkg-config --libs freetype2)
 
 
# SDL3 backend
CFLAGS_SDL3 = -g3 -O0 -I. -I./include -DUI_SDL3
LIBS_SDL3   = -lm
 
# Optional: comment out these two lines to disable FreeType
CFLAGS_SDL3 += -DUI_FREETYPE -DUI_UNICODE $(shell pkg-config --cflags freetype2)
LIBS_SDL3   += $(shell pkg-config --libs freetype2)
 
CFLAGS_SDL3 += $(shell pkg-config --cflags sdl3)
LIBS_SDL3   += $(shell pkg-config --libs sdl3)
 
 
# Examples
EXAMPLES = \
	examples/unit_converter_example.c \
	examples/widget_zoo.c
 
BINS_X11  = $(patsubst examples/%.c, bin/x11/%,  $(EXAMPLES))
BINS_SDL3 = $(patsubst examples/%.c, bin/sdl3/%, $(EXAMPLES))
 
 
# Targets 

.PHONY: all x11 sdl3 clean
 
all: x11
 
x11:  bin/x11  $(BINS_X11)
sdl3: bin/sdl3 $(BINS_SDL3)
 
bin/x11:
	mkdir -p bin/x11
 
bin/sdl3:
	mkdir -p bin/sdl3
 
bin/x11/%: examples/%.c
	$(CC) $(CFLAGS_X11) $< $(LIBS_X11) -o $@
 
bin/sdl3/%: examples/%.c
	$(CC) $(CFLAGS_SDL3) $< $(LIBS_SDL3) -o $@
 
clean:
	rm -rf bin
