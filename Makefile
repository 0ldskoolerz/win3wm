CC      ?= gcc
CFLAGS  ?= -O2 -Wall -Wextra -std=c11
LDFLAGS ?=

RAYLIB_CFLAGS := $(shell pkg-config --cflags raylib 2>/dev/null)
RAYLIB_LIBS   := $(shell pkg-config --libs raylib 2>/dev/null || echo -lraylib)
LUA_CFLAGS    := $(shell pkg-config --cflags lua5.4 2>/dev/null || pkg-config --cflags lua5.3 2>/dev/null || pkg-config --cflags lua 2>/dev/null)
LUA_LIBS      := $(shell pkg-config --libs lua5.4 2>/dev/null || pkg-config --libs lua5.3 2>/dev/null || pkg-config --libs lua 2>/dev/null || echo -llua)

SRC  := src/wm.c src/config.c src/plugins.c
BIN  := win3wm

.PHONY: all clean test

all: $(BIN)

$(BIN): $(SRC) src/main.c src/wm.h src/config.h src/plugins.h
	$(CC) $(CFLAGS) -DWM_WITH_LUA $(RAYLIB_CFLAGS) $(LUA_CFLAGS) \
	      -o $@ src/main.c $(SRC) $(RAYLIB_LIBS) $(LUA_LIBS) -lm

# core-only build: no raylib, no Lua (logic verification)
win3wm-core: $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC)

test: win3wm-core
	./tests/run_tests.sh

clean:
	rm -f $(BIN) win3wm-core tests/test_wm
