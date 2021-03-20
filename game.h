#include "typedef.h"
#include "graphics.h"
#include "camera.h"
#include "engine.h"

#include "scene_obj.h"

void game_keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void game_initFreecamLayount(KeyLayout* layout);


KeyLayout* gameKeyboardLayout;


void gameLoop(Engine engine)
{
	initPredefinedGeometry();
	gameKeyboardLayout = engine.keyLayout;
	game_initFreecamLayount(engine.keyLayout);

	glfwSetKeyCallback(engine.window_ptr, game_keyCallback);

	engine.mainScene = newScene();

	GLint program = newRenderProgram("shaders/base_vertex.vert", "shaders/base_fragment.frag");

	SceneObj* obj = addSceneObj(engine.mainScene, GameObjType);
	obj_setRenderProgram(obj, program);
	obj_setGeometry(obj, &Sprite);

	while (!glfwWindowShouldClose(engine.window_ptr))
	{
		updateFrameTime();

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

		// printf(
		// 	"{%lf, %lf, %lf}\n",
		// 	engine.camera->cameraPos[0],
		// 	engine.camera->cameraPos[1],
		// 	engine.camera->cameraPos[2]
		// );

		renderScene(engine.mainScene);

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

void game_initFreecamLayount(KeyLayout* layout)
{
	keyLayout_bindNewKey(layout, GLFW_KEY_W, MOVE_FORWARD, MOVEMENT_KEY);
	keyLayout_bindNewKey(layout, GLFW_KEY_S, MOVE_BACKWARD, MOVEMENT_KEY);
	keyLayout_bindNewKey(layout, GLFW_KEY_A, MOVE_LEFT, MOVEMENT_KEY);
	keyLayout_bindNewKey(layout, GLFW_KEY_D, MOVE_RIGHT, MOVEMENT_KEY);
}
