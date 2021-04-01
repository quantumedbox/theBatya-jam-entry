#pragma once

#include "graphics.h"
#include "iter.h"
#include "input.h"
#include "camera.h"

void 		initEngine(Engine* engine, uint width, uint height);
Scene* 		newScene();
GameObj* 	newGameObj();
SceneObj* 	addSceneObj(Scene* scene, SceneObjType type);
void 		renderScene(Scene* scene, Camera* camera);
void 		delScene(Scene* scene);
void 		delGameObj(GameObj* obj);
void 		freeEngineResources(Engine* engine);

void 		processAnimations(data_t obj);


void initEngine(Engine* engine, uint width, uint height)
{
	engine->screen_width = width;
	engine->screen_height = height;

	engine->camera = (Camera*)malloc(sizeof(Camera));
	cam_init(engine->camera);
	cam_updatePerspective(engine->camera, (float)width / height);

	engine->keyLayout = keyLayout_new();
	engine->mainScene = newScene();
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

	// logf("adding scene_obj at %p to scene %p\n", new, scene->objs);
	addIter(scene->objs, new, ON_HEAP);

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
	// logf("deleting scene at %p\n", scene);

	Iterator* iter = getIterator(scene->objs);
	while_iter(iter)
	{
		SceneObj* obj = next_iteration_of_type(iter, SceneObj);
		check_stop_iteration(obj);

		// logf("freeing scene_obj at %p\n", obj);

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
	}

	free(scene);
}

__forceinline void delGameObj(GameObj* obj)
{
	if (obj->renderObj != NULL)
		delRenderObj(obj->renderObj);
	free(obj);
}

__forceinline void freeEngineResources(Engine* engine)
{
	delScene(engine->mainScene);
}

void renderScene(Scene* scene, Camera* camera)	// TODO Geometry context that is written as view uniform matrix in shader
{
	Iterator* iter = getIterator(scene->objs);

	#ifdef IMPLEMENT_ANIMATIONS
	setIteratorMapFunc(iter, processAnimations);
	#endif

	while_iter(iter)
	{
		SceneObj* obj = next_iteration_of_type(iter, SceneObj);
		check_stop_iteration(obj);

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
	}
}

// IterMapFunc for renderScene
void processAnimations(data_t in)
{
	SceneObj* obj = (SceneObj*)in;
	switch (obj->type) {
	case GameObjType:
		obj->obj->renderObj->frame += (float)timeDelta / 1000 * obj->obj->renderObj->animationSpeed;
		if (obj->obj->renderObj->frame >= obj->obj->renderObj->frameCount)
			obj->obj->renderObj->frame = 0.0f;
		break;

	default:
		return;
	}
}
