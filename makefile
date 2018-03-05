SRC_DIRS  := $(wildcard src/*)
INCLUDES  := $(addprefix -I,$(SRC_DIRS))
SOURCES   := $(foreach sdir,$(SRC_DIRS),$(wildcard $(sdir)/*.cpp))

CC        := g++
SRC       := src/*.cpp $(SOURCES)
COMPILER  := -w -O0 -g -std=c++14
LINKER    := -lSDL2 -lSDL2_image -lSDL2_mixer $(INCLUDES)
OUT       := build/eosos

linux : $(SRC)
	$(CC) $(SRC) $(COMPILER) $(LINKER) -o $(OUT)
