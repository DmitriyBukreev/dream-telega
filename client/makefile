SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=obj/%.o)
CFLAGS = -fsanitize=address -include src/precompiled.h -c
LDFLAGS = -fsanitize=address -o
LIBS = -lpanel -lmenu -lncurses -lm

all: dirs dt-client

dirs:
	@mkdir -p obj bin

clean:
	rm -rf obj precompiled.h.pch

dt-client: precompiled.h.pch $(OBJ)
	gcc $(LDFLAGS) bin/$@ $(OBJ) $(LIBS)
	#------------------------------------------

$(OBJ): obj/%.o: src/%.c
	../check/checkpatch.pl --no-tree --fix -f $^
	#------------------------------------------
	cppcheck $^
	#------------------------------------------
	gcc $(CFLAGS) $^ -o $@
	#------------------------------------------

precompiled.h.pch: src/precompiled.h
	gcc $< -o $@
	#------------------------------------------