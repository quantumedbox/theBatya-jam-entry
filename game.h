#include <stdlib.h>

#include "typedef.h"
#include "graphics.h"
#include "camera.h"
#include "engine.h"
#include "scene_obj.h"

void game_keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void game_cursorPosCallback(GLFWwindow* window, double x, double y);
void game_windowResizeCallback(GLFWwindow* window, int width, int height);
void game_initFreecamLayount(KeyLayout* layout);


Engine* gameEngine;


// TODO Нужно думать, каким именно образом разделять клиент и сервер

void gameLoop(Engine engine)
{
	glfwSetInputMode(engine.window_ptr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	initPredefinedGeometry();

	gameEngine = &engine;

	game_initFreecamLayount(engine.keyLayout);

	glfwSetKeyCallback(engine.window_ptr, game_keyCallback);
	glfwSetCursorPosCallback(engine.window_ptr, game_cursorPosCallback);
	glfwSetWindowSizeCallback(engine.window_ptr, game_windowResizeCallback);

	GLint program = newRenderProgram("shaders/base_vertex.vert", "shaders/base_fragment.frag");
	TextureObj* fire_texture = newTextureObj("assets/Fire16x.png", 16, 16);
	// TextureObj* table_texture = newTextureObj("assets/table.gif", 0, 0);

	SceneObj* scene = addSceneObj(engine.mainScene, NestedSceneType);
	// SceneObj* table = addSceneObj(engine.mainScene, GameObjType);
	// obj_setRenderProgram(table, program);
	// obj_setGeometry(table, &Sprite);
	// obj_setTextureObj(table, table_texture);
	// obj_setScalePlaneRelative(table, 10.f);

	for (int i = 25; i--;)
	{
		SceneObj* obj = addSceneObj(scene->scene, GameObjType);
		// TODO State machine for single declaration of obj properties for the all new
		obj_setRenderProgram(obj, program);
		obj_setGeometry(obj, &Sprite);
		obj_setPosition(obj, (vec3){rand()%25, 0, rand()%25});
		obj_setTextureObj(obj, fire_texture);
		obj_setFrame(obj, rand()%9);
	}

	while (!glfwWindowShouldClose(engine.window_ptr))
	{
		updateFrameTime();

		scene->scene->position[0] -= 0.0001;

		glClearColor(
			engine.graphicsPref.clearColor[0],
			engine.graphicsPref.clearColor[1],
			engine.graphicsPref.clearColor[2], 1.0f
		);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cam_updateDirection(engine.camera);
		cam_updateLookAt(engine.camera);

		cam_processInput(engine.camera);
		cam_processMovement(engine.camera);
	
		engine.camera->cameraPos[1] = 0;

		renderScene(engine.mainScene, engine.camera);

		glfwSwapBuffers(engine.window_ptr);
		glfwPollEvents();
	}

	freeEngineResources(&engine);
}

#define CLOSE_WINDOW 0xF0

void game_keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	KeyBinding* bind = keyLayout_processKey(gameEngine->keyLayout, key);
	if (bind == NULL)
		return;

	switch (bind->type) {
	case MOVEMENT_KEY:
		if (action == GLFW_PRESS) cam_setInputState(bind->action, true);
		if (action == GLFW_RELEASE) cam_setInputState(bind->action, false);
		break;
	case CONTROL_KEY:
		if (bind->action == CLOSE_WINDOW) glfwSetWindowShouldClose(window, true);
		break;
	case UNKNOWN_KEY:
		WARNING(UNKNOWN_KEY_WARN);
	}
}

void game_cursorPosCallback(GLFWwindow* window, double x, double y)
{
	static double last_x;
	static double last_y;

	cam_updateCursor(gameEngine->camera, x - last_x, y - last_y);
	last_x = x;
	last_y = y;
}

void game_windowResizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	cam_updatePerspective(gameEngine->camera, (float)width / height);
	gameEngine->screen_width = width;
	gameEngine->screen_height = height;
}

void game_initFreecamLayount(KeyLayout* layout)
{
	keyLayout_bindNewKey(layout, GLFW_KEY_W, MOVE_FORWARD, MOVEMENT_KEY);
	keyLayout_bindNewKey(layout, GLFW_KEY_S, MOVE_BACKWARD, MOVEMENT_KEY);
	keyLayout_bindNewKey(layout, GLFW_KEY_A, MOVE_LEFT, MOVEMENT_KEY);
	keyLayout_bindNewKey(layout, GLFW_KEY_D, MOVE_RIGHT, MOVEMENT_KEY);

	keyLayout_bindNewKey(layout, GLFW_KEY_ESCAPE, CLOSE_WINDOW, CONTROL_KEY);
}
