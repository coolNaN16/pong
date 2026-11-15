SDL2_DIR     = /opt/sdl2_mingw/SDL2-2.0.22/x86_64-w64-mingw32
SDL2_TTF_DIR = /opt/sdl2_mingw/SDL2_ttf-2.24.0/x86_64-w64-mingw32
SDL2_IMG_DIR = /opt/sdl2_mingw/SDL2_image-2.8.12/x86_64-w64-mingw32

CFLAGS  = -I$(SDL2_DIR)/include -I$(SDL2_DIR)/include/SDL2 -I$(SDL2_TTF_DIR)/include -I$(SDL2_IMG_DIR)/include
LDFLAGS = -L$(SDL2_DIR)/lib -L$(SDL2_TTF_DIR)/lib -L$(SDL2_IMG_DIR)/lib \
          -lmingw32 -lSDL2main -lSDL2_ttf -lSDL2_image -lSDL2 -mwindows -static-libgcc

build:
	x86_64-w64-mingw32-gcc ../src/main.c -o pong.exe $(CFLAGS) $(LDFLAGS)
	cp $(SDL2_DIR)/bin/*.dll $(SDL2_TTF_DIR)/bin/*.dll $(SDL2_IMG_DIR)/bin/*.dll .
