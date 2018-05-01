SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=obj/%.o)
CFLAGS = -fsanitize=address -g -Wall -include src/precompiled.h -c
LDFLAGS = -fsanitize=address -g -o
LIBS = -lpthread -D_REENTRANT

all: dirs dt-server

dirs:
	@mkdir -p obj bin bin/topics

clean:
	rm -rf obj precompiled.h.pch

dt-server: precompiled.h.pch $(OBJ)
	gcc $(LDFLAGS) bin/$@ $(OBJ) $(LIBS)
	#------------------------------------------

$(OBJ): obj/%.o: src/%.c
	../check/checkpatch.pl --no-tree -f $^
	#------------------------------------------
	cppcheck $^ --enable=performance,portability,warning,style

	#------------------------------------------
	gcc $(CFLAGS) $^ -o $@
	#------------------------------------------

precompiled.h.pch: src/precompiled.h
	gcc $< -o $@
	#------------------------------------------
