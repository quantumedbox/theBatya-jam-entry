#pragma once

#include "typedef.h"

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
	// 2 vertex positions, 2 texture coords follow
	VT2_TX2,
}
PrimitiveDataLayout;

typedef struct
{
	GLfloat* vertexData;
	PrimitiveDataLayout vertexLayout;
	uint sizeOfData;
}
Primitive;

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
	GLfloat* vertexData;
	TextureObj* textureObj;
	uint frame;
}
RenderObj;

RenderObj* newRenderObj()
{
	RenderObj* new = (RenderObj*)malloc(sizeof(RenderObj));

	return new;
}
