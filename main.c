#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "freetype.h"
#include <cglm/cglm.h>

// TODO define clearly all debug and release options
// #define RELEASE
#ifdef DEBUG
//#	define LOG_IN_FILE	// TODO define it to save all the log in the file rather than print on console
#	define logf(fmt, ...) 	fprintf(stderr, fmt, __VA_ARGS__)	// TODO should be replaced with fprintf to stderr/stdout
#	define logln(text)		fprintf(stderr, text)
#	define STRICT_RUNTIME	// don't ignore any errors
#endif

#ifdef RELEASE
#	define logf(...) 	//
#	define logln(...)	//
#endif

const char* afterText = "Tranks for playing!\n"\
"Now this thing is not a 2 day jam project anymore...\n"\
"But code is still available at github!\n"\
"https://github.com/quantumedbox/theBatya-jam-entry\n\n"\
"press any key to close this prompt...";

#include "errors.h"
#include "typedef.h"

#include "submodules/networking/networking.h"
#include "submodules/networking/client.h"
#include "submodules/networking/server.h"

#include "elapsed_time.h"

#ifdef CLIENT_BUILD	// Server build should not include all the graphical fluff
#	define IMPLEMENT_GRAPHICS
#	define IMPLEMENT_ANIMATIONS
#endif

#include "game.h"
#include "voiddelfunc.h"
#include "map.h"	// temporal

GLFWwindow* initScreen(uint width, uint height);
void 		initOpenGL(void);
void 		appClosure(void);

// OPTIONS
_Bool OPT_DONOTEXIT;
_Bool OPT_NOHOST;

void dissectArgs(int argc, const char** argv)
{
	#define arg_is(str) !strcmp(argv[i], str)

	for (int i = 0; i < argc; i++)
	{
		if (arg_is("-donotexit"))
		{
			OPT_DONOTEXIT = true;
		}
		else if (arg_is("-nohost"))
		{
			OPT_NOHOST = true;
		}
	}
}

int main(int argc, const char** argv)
{
	Map* map = mapNew();

	getProfileTime();
	for (int i = 1000000; i--;)
	{
		GameObj* obj = newGameObj();
		mapAddByFuncHeap(map, hashAddress, obj, delGameObj);
		// mapClear(map);
	}
	mapClear(map);

	printProfileTime("iter score");

	mapPrint(map);

	dissectArgs(argc, argv);

	#ifdef CLIENT_BUILD
	if (!OPT_NOHOST)
	{
		ClientAPI* client;
		initWSA();
		client = newClientAPI();
		if (!clientConnect(client, LOCALHOST, DEFAULT_LISTENING_PORT))
			exit('!');
	}
	#endif
	#ifdef SERVER_BUILD
		ServerAPI* server;

		initWSA();
		server = newServerAPI();
		initServer(server, LOCALHOST, DEFAULT_LISTENING_PORT, DEFAULT_ANSWERING_PORT);

		printServerInfo(server);
	#endif

	Engine gameEngine;
	initEngine(&gameEngine, 600, 600);

	#ifdef IMPLEMENT_GRAPHICS	// TODO graphic-less build
	// Defaults:	// why is it here tho?
	gameEngine.graphicsPref.clearColor[0] = 0.0f;
	gameEngine.graphicsPref.clearColor[1] = 0.0f;
	gameEngine.graphicsPref.clearColor[2] = 0.0f;

	gameEngine.window_ptr = initScreen(
		gameEngine.screen_width, gameEngine.screen_height
	);

	initOpenGL();

	gameLoop(gameEngine);
	#endif	// TODO Временно сервер не входит в игровой луп

	appClosure();
	return 0;
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
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_MULTISAMPLE);
}

void appClosure(void)
{
	glfwTerminate();
	if (OPT_DONOTEXIT == true)
	{
		printf("press any key to finish...\n");
		getchar();
	}
	else
	{
		#ifdef RELEASE
		printf(afterText);
		getchar();
		#endif
	}
}
