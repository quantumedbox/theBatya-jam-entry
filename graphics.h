#pragma once

#include "typedef.h"
#include "rendering_pipeline.h"
#include "camera.h"

const uint SpriteSize = 30;
const uint SpriteVertexCount = 6;
const GLfloat SpriteVertexData[] = {
// Verticies		// TexCoords
	-0.5, -0.5, 0.0,	0.0, 0.0,
	+0.5, -0.5, 0.0,	1.0, 0.0,
	+0.5, +0.5, 0.0,	1.0, 1.0,
	-0.5, -0.5, 0.0,	0.0, 0.0,
	+0.5, +0.5, 0.0,	1.0, 1.0,
	-0.5, +0.5,	0.0,	0.0, 1.0,
};

typedef enum Layouts {
	// 3 vertex positions,
	// 2 texture coords follow
	VT3_TX2,
}
GeometryDataLayout;

typedef struct
{
	const GLfloat* vertexData;
	GeometryDataLayout vertexLayout;
	uint sizeOfData;
	uint vertexCount;
	GLuint VAO, VBO;
}
Geometry;


Geometry Sprite = {
	.vertexData = SpriteVertexData,
	.vertexLayout = VT3_TX2,
	.sizeOfData = SpriteSize,
	.vertexCount = SpriteVertexCount,
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

void renderObj(RenderObj* obj, vec3 pos, vec3 orientation, Camera* camera)
{
	static Geometry* currentGeometryBind;
	static GLuint currentRenderProgramBind;
	static TextureObj* currentTextureObject;

	static GLint modelMatAddress;
	static GLint viewMatAddress;
	static GLint projectionMatAddress;

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
		// ??? Maybe matrix layout should be fixed for each render program ???
		modelMatAddress = glGetUniformLocation(obj->renderProgram, "model");
		viewMatAddress = glGetUniformLocation(obj->renderProgram, "view");
		projectionMatAddress = glGetUniformLocation(obj->renderProgram, "projection");
		OPENGL_CHECK("(renderObj) Cannot get matrix uniform locations");
	}

	if ((currentTextureObject != obj->textureObj || currentTextureObject == NULL) &&
														 obj->textureObj != NULL)
	{
		currentTextureObject = obj->textureObj;
	}

	mat4 modelMat = GLM_MAT4_IDENTITY_INIT;
	glm_translate(modelMat, pos);
	// TODO Rotation to orientation vector

	glUniformMatrix4fv(viewMatAddress, 1, GL_FALSE, camera->view[0]);
	glUniformMatrix4fv(projectionMatAddress, 1, GL_FALSE, camera->projection[0]);
	glUniformMatrix4fv(modelMatAddress, 1, GL_FALSE, modelMat[0]);

	glDrawArrays(GL_TRIANGLES, 0, obj->geometry->vertexCount);
	OPENGL_CHECK("(renderObj) While drawing triangles from buffers");
}

void renderObjRelativeTo(RenderObj* obj, vec3 relative_to, vec3 pos, vec3 orientation, Camera* camera)
{
	vec3 absolutePos = GLM_VEC3_ZERO_INIT;
	glm_vec3_add(relative_to, pos, absolutePos);
	renderObj(obj, absolutePos, orientation, camera);
}

__forceinline void delRenderObj(RenderObj* obj)
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

		_PREDEFINED_GEOMETRY[i].VAO = VAO;
		_PREDEFINED_GEOMETRY[i].VBO = VBO;

		switch (_PREDEFINED_GEOMETRY[i].vertexLayout) {
		case VT3_TX2:
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, _PREDEFINED_GEOMETRY[i].sizeOfData / _PREDEFINED_GEOMETRY[i].vertexCount * sizeof(GLfloat), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, _PREDEFINED_GEOMETRY[i].sizeOfData / _PREDEFINED_GEOMETRY[i].vertexCount * sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));

			glBufferData(GL_ARRAY_BUFFER, _PREDEFINED_GEOMETRY[i].sizeOfData * sizeof(GLfloat), _PREDEFINED_GEOMETRY[i].vertexData, GL_STATIC_DRAW);
			break;
		}
		OPENGL_CHECK("(initPredefinedGeometry) When binding buffers for predefined geometry types");
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
