WIN_CC := x86_64-w64-mingw32-gcc

LDFLAGS_LINUX := -Iexternal/raylib-5.0_linux_amd64/include/ -Lexternal/raylib-5.0_linux_amd64/lib/ -l:libraylib.a -lGL -lm -pthread -ldl -lrt -lX11 -l:libblas.a
LDFLAGS_WINDOWS := -Iexternal/raylib-5.0_win64_mingw-w64/include/ -Lexternal/raylib-5.0_win64_mingw-w64/lib/ -Iexternal/OpenBLAS-0.3.24-x64/include -Lexternal/OpenBLAS-0.3.24-x64/lib/ -lraylib -lwinmm -lgdi32 -lopenblas -static
CFLAGS := -Wall -Wno-implicit -Wno-multistatement-macros -O3

BUILD_DIR := ./build

neural_net_lib_linux := $(BUILD_DIR)/lib/ocrc/lib/neural_net.o
neural_net_lib_windows := $(BUILD_DIR)/lib/ocrc/lib/x86_64-w64-neural_net.o

linkedlist_lib_linux := $(BUILD_DIR)/lib/linkedlist/lib/linkedlist.o
linkedlist_lib_windows := $(BUILD_DIR)/lib/linkedlist/lib/x86_64-w64-linkedlist.o

PROGRAMS := editor x86_64-w64-editor.exe battle_horizon x86_64-w64-battle_horizon.exe
BINS := $(PROGRAMS:%=$(BUILD_DIR)/%)

all: $(neural_net_lib_linux) $(neural_net_lib_windows) $(linkedlist_lib_linux) $(linkedlist_lib_windows) $(BINS)

$(neural_net_lib_linux): lib/ocrc/src/neural_net.c lib/ocrc/src/model.h
	mkdir -p $(BUILD_DIR)/lib/ocrc/lib
	$(CC) -c $< -o $@ $(CFLAGS)
$(neural_net_lib_windows): lib/ocrc/src/neural_net.c lib/ocrc/src/model.h
	mkdir -p $(BUILD_DIR)/lib/ocrc/lib
	$(WIN_CC) -c $< -o $@ $(CFLAGS) -Iexternal/OpenBLAS-0.3.24-x64/include -Lexternal/OpenBLAS-0.3.24-x64/lib/

$(linkedlist_lib_linux): lib/linkedlist/linkedlist.c lib/linkedlist/linkedlist.h
	mkdir -p $(BUILD_DIR)/lib/linkedlist/lib
	$(CC) -c $< -o $@ $(CFLAGS)
$(linkedlist_lib_windows): lib/linkedlist/linkedlist.c lib/linkedlist/linkedlist.h
	mkdir -p $(BUILD_DIR)/lib/linkedlist/lib
	$(WIN_CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/editor: src/editor.c src/battle_horizon.c include/defs.h src/camera.c $(linkedlist_lib_linux)
	mkdir -p $(BUILD_DIR)
	$(CC) $< $(linkedlist_lib_linux) -o $@ $(LDFLAGS_LINUX) $(CFLAGS)
$(BUILD_DIR)/x86_64-w64-editor.exe: src/editor.c src/battle_horizon.c include/defs.h src/camera.c $(linkedlist_lib_windows)
	mkdir -p $(BUILD_DIR)
	$(WIN_CC) $< $(linkedlist_lib_windows) -o $@ $(LDFLAGS_WINDOWS) $(CFLAGS)

$(BUILD_DIR)/battle_horizon: src/game.c src/battle_horizon.c include/defs.h src/ui.c include/translate.h src/camera.c lib/arrays/array.c $(neural_net_lib_linux) $(linkedlist_lib_linux)
	mkdir -p $(BUILD_DIR)
	$(CC) $< $(neural_net_lib_linux) $(linkedlist_lib_linux) -o $@ $(LDFLAGS_LINUX) $(CFLAGS)
$(BUILD_DIR)/x86_64-w64-battle_horizon.exe: src/game.c src/battle_horizon.c include/defs.h src/ui.c include/translate.h src/camera.c lib/arrays/array.c $(neural_net_lib_windows) $(linkedlist_lib_windows)
	mkdir -p $(BUILD_DIR)
	$(WIN_CC) $< $(neural_net_lib_windows) $(linkedlist_lib_windows) -o $@ $(LDFLAGS_WINDOWS) $(CFLAGS)

clean:
	rm -rd $(BUILD_DIR)
