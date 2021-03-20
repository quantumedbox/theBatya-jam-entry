#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

// #define RELEASE
#define DEBUG
// #define LOG_IN_FILE	// TODO define it to save all the log in the file rather than print on console

#define logf(fmt, ...) printf(fmt, __VA_ARGS__);

#ifdef RELEASE
#undef logf
#define logf //
#define STRICT_RUNTIME	// don't ignore any errors
#endif

const char* afterText = "Thanks for playing!\n"\
"The game was fully written in just 2 days in pure C!\n"\
"BTW, Source code is freely available on github:\n"\
"https://github.com/quantumedbox/theBatya-jam-entry\n\n"\
"press any key to close this prompt...";

#include "errors.h"
#include "typedef.h"

#include "game.h"

GLFWwindow* initScreen(uint width, uint height);
void initOpenGL(void);
void appClosure(void);

int main(void)
{
	Engine gameEngine;
	initEngine(&gameEngine, 600, 600);

	// Defaults:
	gameEngine.graphicsPref.clearColor[0] = 0.0f;
	gameEngine.graphicsPref.clearColor[1] = 0.0f;
	gameEngine.graphicsPref.clearColor[2] = 0.0f;

	gameEngine.window_ptr = initScreen(
		gameEngine.screen_width, gameEngine.screen_height
	);

	initOpenGL();

	gameLoop(gameEngine);

	appClosure();
}

GLFWwindow* initScreen(uint width, uint height)
{
	GLFWwindow* window;

	if (!glfwInit()) {
		EXIT_ERROR(GLFW_INIT_ERR);
	}

	window = glfwCreateWindow(width, height, "zavicimost", NULL, NULL);
	
	if (!window) {
		glfwTerminate();
		EXIT_ERROR(WINDOW_CREATION_ERR);
	}

	glfwMakeContextCurrent(window);
	return window;
}

void initOpenGL(void)
{
	GLenum err;
	if ((err = glewInit()) != GLEW_OK) {
		OPENGL_ERROR(GLEW_INIT_ERR, err);
	}
}

void appClosure(void)
{
	glfwTerminate();
	#ifdef RELEASE
		printf(afterText);
		getchar();
	#endif
}
