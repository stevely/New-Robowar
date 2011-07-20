# Makefile for development version of NewRobowar

CC= clang
CFLAGS= -g -W -Wall -Werror
LIBS=

# Build target directory
DEST=../bin

# Compiler
RW_C= robocompiler
RW_C_S= compiler.c robocompiler.c robotfile.c
RW_C_N= compiler

# Battle sources
RW_B_S= battlehandler.c codeexecution.c projectiles.c robomath.c robotfile.c

# Battle test
RW_BT= battletest
RW_BT_S= battletest.c

# Graphics test
RW_G= graphicstest
RW_G_S= graphicstest.c

RW_G_FLAGS= `sdl-config --cflags`
RW_G_LIBS= -lpng -lSDL_image -lSDL_ttf `sdl-config --libs`

all:
	@echo "Run make with one of the following:"
	@echo "$(RW_C) $(RW_BT) $(RW_G)"

$(RW_C): $(RW_C_S)
	@echo "Building compiler..."
	$(CC) $(CFLAGS) -o $(DEST)/$@ $? $(LIBS)

$(RW_BT): $(RW_BT_S) $(RW_B_S)
	@echo "Building battle test..."
	$(CC) $(CFLAGS) -o $(DEST)/$@ $? $(LIBS)

$(RW_G): $(RW_G_S) $(RW_B_S)
	@echo "Building graphics test..."
	$(CC) $(CFLAGS) $(RW_G_FLAGS) -o $(DEST)/$@ $? $(LIBS) $(RW_G_LIBS)

.PHONY: none
