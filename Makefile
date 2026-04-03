CC     = gcc
CFLAGS = -g3 -O0 -I. -I./include -DUI_LINUX
LIBS   = -lX11 -lm

# Optional: comment out these two lines to disable FreeType
CFLAGS += -DUI_FREETYPE $(shell pkg-config --cflags freetype2)
LIBS   += $(shell pkg-config --libs freetype2)

EXAMPLES = \
	examples/unit_converter_example.c \
	examples/widget_zoo.c

BINS = $(patsubst examples/%.c, bin/%, $(EXAMPLES))

.PHONY: all clean

all: bin $(BINS)

bin:
	mkdir -p bin

bin/%: examples/%.c
	$(CC) $(CFLAGS) $< $(LIBS) -o $@

clean:
	rm -rf bin
