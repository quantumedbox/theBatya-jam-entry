@echo off
title EngineCompile
gcc -std=c11 submodules/freetype/freetype.c -Wall -o freetype.o -c
gcc -std=c11 main.c -Wall -o build.exe -g -l:glew32.dll -lopengl32 -lglfw3 -lgdi32 -lfreetype -lws2_32
pause
