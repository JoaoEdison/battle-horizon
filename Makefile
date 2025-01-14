LDFLAGS = -Iraylib-5.0_linux_amd64/include/ -Lraylib-5.0_linux_amd64/lib/ -l:libraylib.a -lGL -lm -pthread -ldl -lrt -lX11 -l:libblas.a
CFLAGS = -Wall -Wno-implicit -Wno-multistatement-macros -O3 -g

all: game editor ai/neural_img.o ai/x86_64-w64-neural.o x86_64-w64-battle-horizon.exe

ai/neural_img.o: ai/neural_img.c ai/neural_img.h
	cc -c ai/neural_img.c -o ai/neural_img.o $(CFLAGS)	
game: game.c battle-horizon.c defs.h ui.c translate.c linked_list.c array.c ai/neural_img.o camera.c
	cc game.c ai/neural_img.o -o game $(LDFLAGS) $(CFLAGS) 
editor: editor.c linked_list.c battle-horizon.c defs.h camera.c
	cc editor.c -o editor $(LDFLAGS) $(CFLAGS)
ai/x86_64-w64-neural.o: ai/neural_img.c ai/neural_img.h
	x86_64-w64-mingw32-gcc -c ai/neural_img.c -o ai/x86_64-w64-neural.o -IOpenBLAS-0.3.24-x64/include -LOpenBLAS-0.3.24-x64/lib -static -Wall -Wno-implicit -O3
x86_64-w64-battle-horizon.exe: game.c battle-horizon.c defs.h ui.c translate.c linked_list.c array.c ai/x86_64-w64-neural.o
	x86_64-w64-mingw32-gcc -o x86_64-w64-battle-horizon.exe game.c ai/x86_64-w64-neural.o -Iraylib-5.0_win64_mingw-w64/include/ -Lraylib-5.0_win64_mingw-w64/lib/ -IOpenBLAS-0.3.24-x64/include -LOpenBLAS-0.3.24-x64/lib/ -lraylib -lwinmm -lgdi32 -lopenblas -static $(CFLAGS)

clean:
	rm ai/neural_img.o ai/x86_64-w64-neural.o data/scores.csv
