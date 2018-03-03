SRC_DIRS  := $(wildcard src/*)
INCLUDES  := $(addprefix -I,$(SRC_DIRS))
SOURCES   := $(foreach sdir,$(SRC_DIRS),$(wildcard $(sdir)/*.cpp))

CC        := g++
SRC       := src/*.cpp $(SOURCES)
COMPILER  := -w -O0 -g -std=c++14
LINKER    := -lSDL2 -lSDL2_image -lSDL2_mixer $(INCLUDES)
LUA       := -llua -ldl
OUT       := build/eosos

all : $(SRC)
	$(CC) $(SRC) $(COMPILER) $(LINKER) $(LUA) -o $(OUT)
