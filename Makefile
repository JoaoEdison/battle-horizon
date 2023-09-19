LDFLAGS = -lraylib -lGL -lm -pthread -ldl -lrt -lX11

all: game editor

game: game.c raylib-master/src/rcamera.h spacecraft.c
	sh reinstall.sh
	cc game.c -o game $(LDFLAGS)
editor: editor.c raylib-master/src/rcamera.h linked_list.c spacecraft.c
	sh reinstall.sh
	cc editor.c -o editor $(LDFLAGS) 
