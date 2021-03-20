
// Helper functions for working with SceneObjects

void obj_setRenderProgram(SceneObj* obj, GLuint program)
{
	if (obj->type != GameObjType) {
		WARNING(SETTING_OBJ_OF_INCOMPATIBLE_TYPE_WARN);
		return;
	}
	obj->obj->renderObj->renderProgram = program;
}

void obj_setGeometry(SceneObj* obj, Geometry* geometry)
{
	if (obj->type != GameObjType) {
		WARNING(SETTING_OBJ_OF_INCOMPATIBLE_TYPE_WARN);
		return;
	}
	obj->obj->renderObj->geometry = geometry;
}

void obj_setTextureObj(SceneObj* obj, TextureObj* textureObj)
{
	if (obj->type != GameObjType) {
		WARNING(SETTING_OBJ_OF_INCOMPATIBLE_TYPE_WARN);
		return;
	}
	obj->obj->renderObj->textureObj = textureObj;
}

void obj_setFrame(SceneObj* obj, uint frame)
{
	if (obj->type != GameObjType) {
		WARNING(SETTING_OBJ_OF_INCOMPATIBLE_TYPE_WARN);
		return;
	}
	obj->obj->renderObj->frame = frame;
}

void obj_setPosition(SceneObj* obj, vec3 pos)
{
	if (obj->type == GameObjType) {
		obj->obj->position[0] = pos[0];
		obj->obj->position[1] = pos[1];
		obj->obj->position[2] = pos[2];
	}
	else if (obj->type == NestedSceneType) {
		obj->scene->position[0] = pos[0];
		obj->scene->position[1] = pos[1];
		obj->scene->position[2] = pos[2];
	}
}
