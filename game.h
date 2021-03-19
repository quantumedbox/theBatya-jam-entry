#include "typedef.h"
#include "graphics.h"
#include "engine.h"

void gameLoop(Engine engine)
{
	engine.mainScene = newScene();

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
}
