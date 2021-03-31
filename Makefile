CC=gcc
CFLAGS=-x c -std=c11 -Wall

COMMON_DEPENDENCIES =-lopengl32 -lglfw3 -lgdi32 -lfreetype -lws2_32 -l:glew32.dll

all: server client

server: # freetype # TODO server should not depend on the graphics in the future
	$(CC) $(CFLAGS) main.c -g -o server.exe $(COMMON_DEPENDENCIES) -DSERVER_BUILD=1 -DDEBUG=1

client: # freetype
	$(CC) $(CFLAGS) main.c -g -o client.exe $(COMMON_DEPENDENCIES) -DCLIENT_BUILD=1 -DDEBUG=1

# freetype:
# 	gcc submodules/freetype/freetype.c -o freetype.o -lfreetype-gl

# clean:
# 	rm -rf *.o freetype
