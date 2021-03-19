#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

// #define RELEASE
#define DEBUG

#ifdef RELEASE
#define STRICT_RUNTIME
#endif

#include "errors.h"
#include "typedef.h"

#include "game.h"

GLFWwindow* initScreen(uint width, uint height);
void initOpenGL(void);
void appClosure(void);

int main(void)
{
	Engine gameEngine;
	// Defaults:
	gameEngine.screen_width = 600;
	gameEngine.screen_height = 600;

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
		printf("thanks for playing!");
		getchar();
	#endif
}
