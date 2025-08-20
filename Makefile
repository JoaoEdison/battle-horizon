LDFLAGS_LINUX := -Ilib/raylib-5.0_linux_amd64/include/ -Llib/raylib-5.0_linux_amd64/lib/ -l:libraylib.a -lGL -lm -pthread -ldl -lrt -lX11 -l:libblas.a
LDFLAGS_WINDOWS := -Ilib/raylib-5.0_win64_mingw-w64/include/ -Llib/raylib-5.0_win64_mingw-w64/lib/ -Ilib/OpenBLAS-0.3.24-x64/include -Llib/OpenBLAS-0.3.24-x64/lib/ -lraylib -lwinmm -lgdi32 -lopenblas -static
CFLAGS := -Wall -Wno-implicit -Wno-multistatement-macros -O3 -g

BUILD_DIR := ./build

neural_net_lib_linux := $(BUILD_DIR)/lib/ocrc/lib/neural_net.o
neural_net_lib_windows := $(BUILD_DIR)/lib/ocrc/lib/x86_64-w64-neural_net.o

PROGRAMS := editor x86_64-w64-editor.exe battle_horizon x86_64-w64-battle_horizon.exe
BINS := $(PROGRAMS:%=$(BUILD_DIR)/%)

all: $(neural_net_lib_linux) $(neural_net_lib_windows) $(BINS)

$(neural_net_lib_linux): lib/ocrc/src/neural_net.c lib/ocrc/src/model.h
	mkdir -p $(BUILD_DIR)/lib/ocrc/lib
	$(CC) -c $< -o $@ $(CFLAGS)
$(neural_net_lib_windows): lib/ocrc/src/neural_net.c lib/ocrc/src/model.h
	mkdir -p $(BUILD_DIR)/lib/ocrc/lib
	x86_64-w64-mingw32-gcc -c $< -o $@ $(CFLAGS) -Ilib/OpenBLAS-0.3.24-x64/include -Llib/OpenBLAS-0.3.24-x64/lib/

$(BUILD_DIR)/editor: src/editor.c src/linked_list.c src/battle_horizon.c src/defs.h src/camera.c
	mkdir -p $(BUILD_DIR)
	$(CC) $< -o $@ $(LDFLAGS_LINUX) $(CFLAGS)
$(BUILD_DIR)/x86_64-w64-editor.exe: src/editor.c src/linked_list.c src/battle_horizon.c src/defs.h src/camera.c
	mkdir -p $(BUILD_DIR)
	x86_64-w64-mingw32-gcc $< -o $@ $(LDFLAGS_WINDOWS) $(CFLAGS)

$(BUILD_DIR)/battle_horizon: src/game.c src/battle_horizon.c src/defs.h src/ui.c src/translate.c src/camera.c src/linked_list.c lib/arrays/array.c $(neural_net_lib_linux)
	mkdir -p $(BUILD_DIR)
	$(CC) $< $(neural_net_lib_linux) -o $@ $(LDFLAGS_LINUX) $(CFLAGS)
$(BUILD_DIR)/x86_64-w64-battle_horizon.exe: src/game.c src/battle_horizon.c src/defs.h src/ui.c src/translate.c src/camera.c src/linked_list.c lib/arrays/array.c $(neural_net_lib_windows)
	mkdir -p $(BUILD_DIR)
	x86_64-w64-mingw32-gcc $< $(neural_net_lib_windows) -o $@ $(LDFLAGS_WINDOWS) $(CFLAGS)

clean:
	rm -rd $(BUILD_DIR)
