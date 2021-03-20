
// Helper functions for working with SceneObjects

void obj_setRenderProgram(SceneObj* obj, GLuint program)
{
	obj->obj->renderObj->renderProgram = program;
}

void obj_setGeometry(SceneObj* obj, Geometry* geometry)
{
	obj->obj->renderObj->geometry = geometry;
}

void obj_setTextureObj(SceneObj* obj, TextureObj* textureObj)
{
	obj->obj->renderObj->textureObj = textureObj;
}

void obj_setFrame(SceneObj* obj, uint frame)
{
	obj->obj->renderObj->frame = frame;
}
