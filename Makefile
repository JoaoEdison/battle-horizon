all: game editor

game: spacecraft.c raylib-master/src/rcamera.h
	sh reinstall.sh
	cc spacecraft.c -o game -lraylib -lGL -lm -pthread -ldl -lrt -lX11
editor: editor.c raylib-master/src/rcamera.h linked_list.c
	sh reinstall.sh
	cc editor.c -o editor -lraylib -lGL -lm -pthread -ldl -lrt -lX11
