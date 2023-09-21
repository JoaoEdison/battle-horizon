LDFLAGS = -lraylib -lGL -lm -pthread -ldl -lrt -lX11 -g

all: game editor

#retirar a dependencia com a libpng

game: game.c raylib-master/src/rcamera.h spacecraft.c defs.h
	sh reinstall.sh
	cc game.c ia/neural_img.o -o game $(LDFLAGS) -lpng -lcblas
editor: editor.c raylib-master/src/rcamera.h linked_list.c spacecraft.c defs.h
	sh reinstall.sh
	cc editor.c -o editor $(LDFLAGS)
