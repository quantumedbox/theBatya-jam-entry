#pragma once

#include "graphics.h"
#include "iter.h"
#include "input.h"

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
	Iter* iter;				// All game objects that should be processed
}
Scene;

typedef struct
{
	RenderObj* renderObj;	// NULL if not rendered
	vec3 pos;
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
	KeyLayout* KeyLayout;
	Scene* mainScene;	// Entry point for game object login
}
Engine;

Scene* newScene()
{
	Scene* new = (Scene*)malloc(sizeof(Scene));
	new->iter = (Iter*)malloc(sizeof(Iter));

	return new;
}

void addSceneObj(Scene* scene, SceneObjType type)
{
	SceneObj* new = (SceneObj*)malloc(sizeof(SceneObj));
	new->type = type;
	switch (type) {
	case NestedSceneType:
		new->scene = newScene();
		break;
	case GameObjType:
		new->obj->renderObj = newRenderObj();
		break;
	case LogicType:
		// TODO
		break;
	default:
		WARNING(CREATED_UNKNOWN_SCENEOBJ_TYPE);
	}
}
