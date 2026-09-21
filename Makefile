# Resonance - build with GCC and GTK 3 (Linux or MSYS2 on Windows)
#   make           build bin/resonance
#   make run       build and run (the program uses paths relative to bin/)
#   make database  rebuild files/musics_database.bin from data/musics.txt
#   make clean     remove build output

CC      = gcc
CFLAGS  += -Wall -Wextra -O2 $(shell pkg-config --cflags gtk+-3.0)
LDLIBS  += $(shell pkg-config --libs gtk+-3.0) -lm

EXE :=
ifeq ($(OS),Windows_NT)
    EXE     := .exe
    LDFLAGS += -mwindows
endif

TARGET  := bin/resonance$(EXE)
TOOL    := bin/build_database$(EXE)

SRC := src/main.c src/library.c
HDR := src/library.h

DATA_SRC := data/musics.txt
DATABASE := files/musics_database.bin

.PHONY: all run database clean

all: $(TARGET)

$(TARGET): $(SRC) $(HDR)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LDFLAGS) $(LDLIBS)

run: $(TARGET)
	cd bin && ./$(notdir $(TARGET))

# The tool is plain C and does not need GTK.
$(TOOL): tools/build_database.c $(HDR)
	@mkdir -p bin
	$(CC) -Wall -Wextra -O2 -o $@ tools/build_database.c

database: $(TOOL)
	./$(TOOL) $(DATA_SRC) $(DATABASE)

clean:
	rm -f $(TARGET) $(TOOL)
