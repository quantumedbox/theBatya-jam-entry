#pragma once

#include "typedef.h"

#include "shader_handler.h"
// #include "engine.h"

const uint SpriteSize = 24;
const GLfloat Sprite[] = {
	// Verticies	// Texture coords
	-0.5, -0.5,		0.0, 0.0,
	+0.5, -0.5,		1.0, 0.0,
	+0.5, +0.5,		1.0, 1.0,
	-0.5, -0.5,		0.0, 0.0,
	+0.5, +0.5,		1.0, 1.0,
	-0.5, +0.5,		0.0, 1.0,
};

typedef enum Layouts {
	// 2 vertex positions,
	// 2 texture coords follow
	VT2_TX2,
}
GeometryDataLayout;

typedef struct
{
	GLfloat* vertexData;
	GeometryDataLayout vertexLayout;
	uint sizeOfData;
}
Geometry;

// Stores texture sheet data for sprites
typedef struct
{
	GLuint texture;	// Pointer to GPU buffer
	uvec2 size;		// Width / Height in texels
	uvec2 subSize;	// Width / Height for each subtexture
}
TextureObj;

typedef struct
{
	GLuint renderProgram;
	Geometry* geometry;
	TextureObj* textureObj;
	uint frame;
}
RenderObj;

RenderObj* newRenderObj()
{
	RenderObj* new = (RenderObj*)malloc(sizeof(RenderObj));
	new->renderProgram = 0;
	new->geometry = NULL;
	new->textureObj = NULL;
	new->frame = 0;

	return new;
}

void delRenderObj(RenderObj* obj)
{
	free(obj->textureObj);
	free(obj);
}
