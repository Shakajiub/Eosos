CC        := g++
LD        := g++

MODULES   := ability actor algorithm engine scene scene/menu scene/scenario sound texture ui
COMPILER  := -w -O0 -g -std=c++14
LINKER    := -lSDL2 -lSDL2_image -lSDL2_mixer

SRC_DIRS  := $(addprefix src/,$(MODULES)) src
BLD_DIRS  := $(addprefix obj/,$(MODULES)) obj

SRC       := $(foreach sdir,$(SRC_DIRS),$(wildcard $(sdir)/*.cpp))
OBJ       := $(patsubst src/%.cpp,obj/%.o,$(SRC))
INCLUDES  := $(addprefix -I,$(SRC_DIRS))

vpath %.cpp $(SRC_DIRS)

define make-goal
$1/%.o: %.cpp
	$(CC) $(COMPILER) $(INCLUDES) -c $$< -o $$@
endef

.PHONY: all checkdirs clean

all: checkdirs build/eosos

build/eosos: $(OBJ)
	$(LD) $^ -o $@ $(LINKER)

checkdirs: $(BLD_DIRS)

$(BLD_DIRS):
	@mkdir -p $@

clean:
	@rm -rf $(BLD_DIRS)

$(foreach bdir,$(BLD_DIRS),$(eval $(call make-goal,$(bdir))))
