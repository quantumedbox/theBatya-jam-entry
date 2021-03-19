#include "typedef.h"
#include "graphics.h"
#include "engine.h"

#include "scene_obj.h"

void gameLoop(Engine engine)
{
	engine.mainScene = newScene();

	GLint program = initRenderProgram("shaders/base_vertex.vert", "shaders/base_fragment.frag");

	SceneObj* obj = addSceneObj(engine.mainScene, GameObjType);
	obj_setRenderProgram(obj, program);

	while (!glfwWindowShouldClose(engine.window_ptr))
	{
		glClearColor(
			engine.graphicsPref.clearColor[0],
			engine.graphicsPref.clearColor[1],
			engine.graphicsPref.clearColor[2], 1.0f
		);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glfwSwapBuffers(engine.window_ptr);
		glfwPollEvents();
	}

	freeEngineResources(&engine);
}
