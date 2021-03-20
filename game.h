#include "typedef.h"
#include "graphics.h"
#include "camera.h"
#include "engine.h"

#include "scene_obj.h"

#include <stdlib.h>

void game_keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void game_cursorPosCallback(GLFWwindow* window, double x, double y);
void game_windowResizeCallback(GLFWwindow* window, int width, int height);
void game_initFreecamLayount(KeyLayout* layout);


KeyLayout* 	gameKeyboardLayout;
Camera*		gameCamera;


void gameLoop(Engine engine)
{
	initPredefinedGeometry();

	gameKeyboardLayout = engine.keyLayout;
	gameCamera = engine.camera;

	game_initFreecamLayount(engine.keyLayout);

	glfwSetKeyCallback(engine.window_ptr, game_keyCallback);
	glfwSetCursorPosCallback(engine.window_ptr, game_cursorPosCallback);
	glfwSetWindowSizeCallback(engine.window_ptr, game_windowResizeCallback);

	engine.mainScene = newScene();

	GLint program = newRenderProgram("shaders/base_vertex.vert", "shaders/base_fragment.frag");

	SceneObj* scene = addSceneObj(engine.mainScene, NestedSceneType);
	SceneObj* obj = addSceneObj(scene->scene, GameObjType);
	obj_setRenderProgram(obj, program);
	obj_setGeometry(obj, &Sprite);
	obj_setPosition(scene, (vec3){0.0, 0.0, 0.0});

	while (!glfwWindowShouldClose(engine.window_ptr))
	{
		updateFrameTime();

		obj->obj->position[1] -= 0.0001;

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

void game_keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	KeyBinding* bind = keyLayout_processKey(gameKeyboardLayout, key);
	if (bind == NULL)
		return;

	switch (bind->type) {
	case MOVEMENT_KEY:
		if (action == GLFW_PRESS) cam_setInputState(bind->action, true);
		if (action == GLFW_RELEASE) cam_setInputState(bind->action, false);
		break;
	case UNKNOWN_KEY:
		WARNING(UNKNOWN_KEY_WARN);
	}
}

void game_cursorPosCallback(GLFWwindow* window, double x, double y)
{
	static double last_x;
	static double last_y;

	cam_updateCursor(gameCamera, x - last_x, y - last_y);
	last_x = x;
	last_y = y;
}

void game_windowResizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	cam_updatePerspective(gameCamera, (float)width / height);
}

void game_initFreecamLayount(KeyLayout* layout)
{
	keyLayout_bindNewKey(layout, GLFW_KEY_W, MOVE_FORWARD, MOVEMENT_KEY);
	keyLayout_bindNewKey(layout, GLFW_KEY_S, MOVE_BACKWARD, MOVEMENT_KEY);
	keyLayout_bindNewKey(layout, GLFW_KEY_A, MOVE_LEFT, MOVEMENT_KEY);
	keyLayout_bindNewKey(layout, GLFW_KEY_D, MOVE_RIGHT, MOVEMENT_KEY);
}
