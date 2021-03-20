#pragma once

#include "graphics.h"
#include "iter.h"
#include "input.h"
#include "camera.h"

typedef struct {
	uvec3 clearColor;		// Base color of the screen
}
GraphicsPreferences;

typedef enum Type {
	NestedSceneType,
	GameObjType,
	LogicType,
}
SceneObjType;

typedef struct
{
	Iter* objs;				// All game objects that should be processed
	vec3 position;				// Origin point for all object within the scene
}
Scene;

typedef struct
{
	RenderObj* renderObj;	// NULL if not rendered
	vec3 position;
	vec3 orientation;
}
GameObj;

// Abstraction for scane
typedef struct
{
	SceneObjType type;
	union {
		Scene* scene;
		GameObj* obj;
	};
}
SceneObj;

// Game engine object that holds everything needed for the inner workings
typedef struct
{
	GLFWwindow* window_ptr;
	uint screen_width, screen_height;

	GraphicsPreferences graphicsPref;
	KeyLayout* keyLayout;

	Camera* camera;
	Scene* mainScene;	// Entry point for game object login
}
Engine;


void initEngine(Engine* engine, uint width, uint height);
Scene* newScene();
GameObj* newGameObj();
SceneObj* addSceneObj(Scene* scene, SceneObjType type);
void renderScene(Scene* scene, Camera* camera);
void delScene(Scene* scene);
void delGameObj(GameObj* obj);
void freeEngineResources(Engine* engine);


void initEngine(Engine* engine, uint width, uint height)
{
	engine->screen_width = width;
	engine->screen_height = height;

	engine->camera = (Camera*)malloc(sizeof(Camera));
	cam_init(engine->camera);
	cam_updatePerspective(engine->camera, (float)width / height);

	engine->keyLayout = keyLayout_new();
}

__forceinline Scene* newScene()
{
	Scene* new = (Scene*)malloc(sizeof(Scene));
	new->objs = newIter();
	new->position[0] = new->position[1] = new->position[2] = 0;
	return new;
}

SceneObj* addSceneObj(Scene* scene, SceneObjType type)
{
	SceneObj* new = (SceneObj*)malloc(sizeof(SceneObj));
	new->type = type;
	switch (type) {
	case NestedSceneType:
		new->scene = newScene();
		break;
	case GameObjType:
		new->obj = newGameObj();
		break;
	case LogicType:
		// TODO
		break;
	default:
		WARNING(CREATED_UNKNOWN_SCENEOBJ_TYPE);
	}

	logf("adding scene_obj at %p to scene %p\n", new, scene->objs);
	addIter(scene->objs, new);

	return new;
}

__forceinline GameObj* newGameObj()
{
	GameObj* new = (GameObj*)malloc(sizeof(GameObj));
	new->renderObj = newRenderObj();
	new->position[0] = new->position[1] = new->position[2] = 0;
	return new;
}

void delScene(Scene* scene)
{
	logf("deleting scene at %p\n", scene);

	if (lenIter(scene->objs) == 0) {
		free(scene);
		return;
	}
	startIter(scene->objs);
	int remains;
	do {
		SceneObj* obj = (SceneObj*)nextIter(scene->objs);
		logf("freeing scene_obj at %p\n", obj);

		switch (obj->type) {
		case NestedSceneType:
			delScene(obj->scene);
			break;
		case GameObjType:
			delGameObj(obj->obj);
			break;
		case LogicType:
			// TODO
			break;
		default:
			WARNING(UNKNOWN_SCENEOBJ_TYPE);
		}

		remains = remainsIter(scene->objs);
	}
	while (remains != 0);

	free(scene);
}

void delGameObj(GameObj* obj)
{
	if (obj->renderObj != NULL)
		delRenderObj(obj->renderObj);
}

__forceinline void freeEngineResources(Engine* engine)
{
	delScene(engine->mainScene);
}

void renderScene(Scene* scene, Camera* camera)
{
	if (lenIter(scene->objs) == 0)
		return;

	startIter(scene->objs);
	int remains;
	do {
		SceneObj* obj = (SceneObj*)nextIter(scene->objs);

		switch (obj->type) {
		case NestedSceneType:
			renderScene(obj->scene, camera);	// TODO Nested scene's pos should be modified by its parent pos
			break;
		case GameObjType:
			renderObjRelativeTo(
				obj->obj->renderObj,
				scene->position,
				obj->obj->position,
				obj->obj->orientation,
				camera
			);
			break;
		case LogicType:
			break;	// Ignore for rendering
		default:
			WARNING(UNKNOWN_SCENEOBJ_TYPE);
		}

		remains = remainsIter(scene->objs);
	}
	while (remains != 0);
}
