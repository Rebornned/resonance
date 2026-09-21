# Resonance - build with GCC and GTK 3 (Linux or MSYS2 on Windows)
#   make        build bin/resonance
#   make run    build and run (the program uses paths relative to bin/)
#   make clean  remove build output

CC      = gcc
CFLAGS  += -Wall -Wextra -O2 $(shell pkg-config --cflags gtk+-3.0)
LDLIBS  += $(shell pkg-config --libs gtk+-3.0) -lm

TARGET  := bin/resonance
ifeq ($(OS),Windows_NT)
    TARGET  := bin/resonance.exe
    LDFLAGS += -mwindows
endif

SRC := src/main.c src/func.c
HDR := src/playlists.h

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC) $(HDR)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LDFLAGS) $(LDLIBS)

run: $(TARGET)
	cd bin && ./$(notdir $(TARGET))

clean:
	rm -f $(TARGET)
