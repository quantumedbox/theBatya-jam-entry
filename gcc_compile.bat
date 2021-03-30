@echo off
title EngineCompile
gcc -std=c11 -x c submodules/freetype/freetype.c -Wall -O3 -o freetype.o -c
gcc -std=c11 -x c main.c -Wall -g -o build.exe -l:glew32.dll -lopengl32 -lglfw3 -lgdi32 -lfreetype -lws2_32
pause
