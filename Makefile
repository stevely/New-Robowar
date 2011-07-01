# Makefile for development version of NewRobowar

# Source directory
SRC=src

# Compiler
RW_C= robocompiler

# Battle test
RW_BT= battletest

# Graphics test
RW_G= graphicstest

all $(RW_C) $(RW_BT) $(RW_G):
	cd $(SRC) && $(MAKE) $@

.PHONY: all
