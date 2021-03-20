#pragma once

#include "typedef.h"
#include "rendering_pipeline.h"

const uint SpriteSize = 24;
const GLfloat SpriteVertexData[] = {
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
	const GLfloat* vertexData;
	GeometryDataLayout vertexLayout;
	uint sizeOfData;
	GLuint VAO, VBO;
}
Geometry;


Geometry Sprite = {
	.vertexData = SpriteVertexData,
	.vertexLayout = VT2_TX2,
	.sizeOfData = SpriteSize,
};

const uint _PREDEFINED_GEOMETRY_COUNT = 1;
Geometry* _PREDEFINED_GEOMETRY = {
	&Sprite
};


// Stores texture sheet data for sprites
typedef struct
{
	GLuint texture;	// Pointer to GPU buffer
	uvec2 size;		// Width / Height in texels
	uvec2 subSize;	// Width / Height for each subtexture
}
TextureObj;

#define NO_RENDER_PROGRAM 0
typedef struct
{
	GLuint renderProgram;
	Geometry* geometry;
	TextureObj* textureObj;
	uint frame;
}
RenderObj;


RenderObj* newRenderObj(void)
{
	RenderObj* new = (RenderObj*)malloc(sizeof(RenderObj));
	new->renderProgram = NO_RENDER_PROGRAM;
	new->geometry = NULL;
	new->textureObj = NULL;
	new->frame = 0;

	return new;
}

void renderObj(RenderObj* obj, vec3 pos, vec3 orientation)
{
	static Geometry* currentGeometryBind;
	static GLuint currentRenderProgramBind;
	static TextureObj* currentTextureObject;

	if ((currentGeometryBind != obj->geometry || currentGeometryBind == NULL)
													&& obj->geometry != NULL)
	{
		currentGeometryBind = obj->geometry;

		glBindVertexArray(obj->geometry->VAO);
		OPENGL_CHECK("(renderObj) In the proccess of VAO binding");
	}

	if ((currentRenderProgramBind != obj->renderProgram || currentRenderProgramBind == NO_RENDER_PROGRAM) &&
																 obj->renderProgram != NO_RENDER_PROGRAM)
	{
		currentRenderProgramBind = obj->renderProgram;

		glUseProgram(obj->renderProgram);
		OPENGL_CHECK("(renderObj) In the proccess of render program binding");
	}

	if ((currentTextureObject != obj->textureObj || currentTextureObject == NULL) &&
														 obj->textureObj != NULL)
	{
		currentTextureObject = obj->textureObj;
	}

	glDrawArrays(GL_TRIANGLES, 0, obj->geometry->sizeOfData);
	OPENGL_CHECK("(renderObj) While drawing triangles from buffers");
}

void delRenderObj(RenderObj* obj)
{
	// free(obj->textureObj);	// GC should count the amount of references and delete shader objects depending on it
	free(obj);
}

void initPredefinedGeometry(void)
{
	for (uint i = _PREDEFINED_GEOMETRY_COUNT; i--;)
	{
		GLuint VAO, VBO;
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		switch (_PREDEFINED_GEOMETRY[i].vertexLayout) {
		case VT2_TX2:
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, _PREDEFINED_GEOMETRY[i].sizeOfData * sizeof(GLfloat), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, _PREDEFINED_GEOMETRY[i].sizeOfData * sizeof(GLfloat), (void*)(2*sizeof(GLfloat)));
		}
		OPENGL_CHECK("(initPredefinedGeometry) When binding buffers for predefined geometry types");
	}
}
