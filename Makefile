# for now is only GCC compilation
CC = gcc
CFLAGS = -x c -std=c11 -Wall -fopenmp

COMMON_DEPENDENCIES = -lopengl32 -lglfw3 -lgdi32 -lfreetype -lws2_32 -l:glew32.dll

# defaults to debug server and client
BUILD_OPTION = -DDEBUG=1 -g
all: debug

debug: build_all

release: BUILD_OPTION = -DRELEASE=1
release: build_all

build_all: client server

server: # freetype # TODO server should not depend on the graphics in the future
	$(CC) $(CFLAGS) main.c -o server.exe $(COMMON_DEPENDENCIES) -DSERVER_BUILD=1 $(BUILD_OPTION)

client: # freetype
	$(CC) $(CFLAGS) main.c -o client.exe $(COMMON_DEPENDENCIES) -DCLIENT_BUILD=1 $(BUILD_OPTION)

# freetype:
# 	gcc submodules/freetype/freetype.c -o freetype.o -lfreetype-gl

# clean:
# 	rm -rf *.o freetype
