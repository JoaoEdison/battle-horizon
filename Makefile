LDFLAGS = -lraylib -lGL -lm -pthread -ldl -lrt -lX11 -lcblas -g
CFLAGS = -Wall -Wno-implicit -Wno-multistatement-macros -O3

all: game editor ai/neural_img.o

ai/neural_img.o: ai/neural_img.c ai/neural_img.h
	cc -c ai/neural_img.c -o ai/neural_img.o $(CFLAGS)	
game: game.c raylib-master/src/rcamera.h spacecraft.c defs.h ai/neural_img.o ui.c
	sh reinstall.sh
	cc game.c ai/neural_img.o -o game $(LDFLAGS) $(CFLAGS) 
editor: editor.c raylib-master/src/rcamera.h linked_list.c spacecraft.c defs.h
	sh reinstall.sh
	cc editor.c -o editor $(LDFLAGS) $(CFLAGS)
