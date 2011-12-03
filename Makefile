# Makefile for development version of NewRobowar

CC= clang
ANALYZER= clang --analyze
CFLAGS= -g -W -Wall -Werror
LIBS=
SRC= src/

# Compiler
RW_C= robocompiler
RW_C_S= compiler.o robocompiler.o robotfile.o roboconfig.o

# Battle sources
RW_B_S= battlehandler.o codeexecution.o projectiles.o \
        robomath.o robotfile.o tournament.o

# Battle test
RW_BT= battletest
RW_BT_S= battletest.o

# Graphics test
RW_G= graphicstest
RW_G_S= graphicstest.o

RW_G_FLAGS= $(shell sdl-config --cflags)
RW_G_LIBS= -lpng -lSDL_image -lSDL_ttf $(shell sdl-config --libs)

all:
	@echo "Run make with one of the following:"
	@echo "$(RW_C) $(RW_BT) $(RW_G)"

$(RW_C): $(addprefix $(SRC), $(RW_C_S))
	@echo "Building compiler..."
	$(CC) $(CFLAGS) -o $@ $? $(LIBS)

$(RW_BT): $(addprefix $(SRC), $(RW_BT_S) $(RW_B_S))
	@echo "Building battle test..."
	$(CC) $(CFLAGS) -o $@ $? $(LIBS)

$(RW_G): LIBS   += $(RW_G_LIBS)
$(RW_G): CFLAGS += $(RW_G_FLAGS)
$(RW_G): $(addprefix $(SRC), $(RW_G_S) $(RW_B_S))
	@echo "Building graphics test..."
	$(CC) $(CFLAGS) -o $@ $? $(LIBS)

%.o: %.c
	$(ANALYZER) $(CFLAGS) $?
	$(CC) $(CFLAGS) -I $(SRC) -c -o $@ $<

clean:
	rm $(SRC)*.o $(RW_C) $(RW_BT) $(RW_G)

.PHONY: all none clean
