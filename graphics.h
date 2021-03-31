#pragma once

#include "typedef.h"
#include "rendering_pipeline.h"
#include "camera.h"
#include "texture_load.h"

const uint SpriteSize = 30;
const uint SpriteVertexCount = 6;
const GLfloat SpriteVertexData[] = {
  // Verticies		 // TexCoords
	-0.5, -0.5, 0.0,	0.0, 0.0,
	+0.5, -0.5, 0.0,	1.0, 0.0,
	+0.5, +0.5, 0.0,	1.0, 1.0,
	-0.5, -0.5, 0.0,	0.0, 0.0,
	+0.5, +0.5, 0.0,	1.0, 1.0,
	-0.5, +0.5,	0.0,	0.0, 1.0,
};

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

RenderObj* newRenderObj(void);
void renderObj(RenderObj* obj, vec3 pos, vec3 orientation, Camera* camera);
void renderObjRelativeTo(RenderObj* obj, vec3 relative_to, vec3 pos, vec3 orientation, Camera* camera);
__forceinline void getFrameTextureCoords(uint frame, TextureObj* texObj, vec2 write_to);

void initPredefinedGeometry(void);
// void setIdentityMatForRotation(mat4 mat);


RenderObj* newRenderObj(void)
{
	RenderObj* new = (RenderObj*)malloc(sizeof(RenderObj));
	new->renderProgram = NO_RENDER_PROGRAM;
	new->geometry = NULL;
	new->textureObj = NULL;

	new->scale = 1.0f;

	new->frame = 0;
	new->frameCount = 0;
	new->animationSpeed = 1;	// Default is frame per second

	return new;
}

void renderObj(RenderObj* obj, vec3 pos, vec3 orientation, Camera* camera)
{
	// State machine for skipping unnecessary bindings for similar objects
	static Geometry* currentGeometryBind;
	static GLuint currentRenderProgramBind;
	static TextureObj* currentTextureObject;

	static GLint modelMatAddress;
	static GLint viewMatAddress;
	static GLint projectionMatAddress;
	static GLint subtextureCoordsAddress;
	static GLint subtextureSizeAddress;
	static GLint scaleAddress;

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
		OPENGL_CHECK("(renderObj) Cannot bind given render program");

		modelMatAddress 		= glGetUniformLocation(obj->renderProgram, "model");
		viewMatAddress 			= glGetUniformLocation(obj->renderProgram, "view");
		projectionMatAddress 	= glGetUniformLocation(obj->renderProgram, "projection");
		subtextureCoordsAddress = glGetUniformLocation(obj->renderProgram, "subtextureCoords");
		subtextureSizeAddress 	= glGetUniformLocation(obj->renderProgram, "subtextureSize");
		scaleAddress 			= glGetUniformLocation(obj->renderProgram, "scale");
		OPENGL_CHECK("(renderObj) Cannot get shader uniform locations");
	}

	if ((currentTextureObject != obj->textureObj || currentTextureObject == NULL) &&
														 obj->textureObj != NULL)
	{
		currentTextureObject = obj->textureObj;

		glBindTexture(GL_TEXTURE_2D, obj->textureObj->texture_ptr);
		OPENGL_CHECK("(renderObj) Cannot bind texture object");
	}

	mat4 modelMat = GLM_MAT4_IDENTITY_INIT;
	glm_translate(modelMat, pos);

	glUniformMatrix4fv(viewMatAddress, 1, GL_FALSE, camera->view[0]);	// TODO bind single time for each unique render program
	glUniformMatrix4fv(projectionMatAddress, 1, GL_FALSE, camera->projection[0]);	// TODO
	glUniformMatrix4fv(modelMatAddress, 1, GL_FALSE, modelMat[0]);

	vec2 texCoords;
	getFrameTextureCoords((uint)obj->frame, obj->textureObj, texCoords);

	vec2 texSubSize;
	texSubSize[0] = (float)obj->textureObj->subSize[0] / obj->textureObj->size[0];
	texSubSize[1] = (float)obj->textureObj->subSize[1] / obj->textureObj->size[1];

	glUniform2f(subtextureCoordsAddress, texCoords[0], texCoords[1]);
	glUniform2f(subtextureSizeAddress, texSubSize[0], texSubSize[1]);

	glUniform1f(scaleAddress, obj->scale);

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
	// free(obj->textureObj);	// GC-like object should count the amount of references and delete shader objects depending on it
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

// void setIdentityMatForRotation(mat4 mat)
// {
// 	mat[0][0] = 1; mat[0][1] = 0; mat[0][2] = 0; 
// 	mat[0][0] = 0; mat[0][1] = 1; mat[0][2] = 0; 
// 	mat[0][0] = 0; mat[0][1] = 0; mat[0][2] = 1; 
// }

TextureObj* newTextureObj(char* textureDir, uint subWidth, uint subHeight)
{
	TextureObj* new = (TextureObj*)malloc(sizeof(TextureObj));
	populateTextureObjFromFile(new, textureDir);

	if (subWidth == 0)	new->subSize[0] = new->size[0];
	else 				new->subSize[0] = subWidth;
	if (subHeight == 0) new->subSize[1] = new->size[1];
	else 				new->subSize[1] = subHeight;

	return new;
}

__forceinline void getFrameTextureCoords(uint frame, TextureObj* texObj, vec2 write_to)
{
	uint width = texObj->size[0] / texObj->subSize[0];
	// uint height = texObj->size[1] / texObj->subSize[1];

	int x = frame % width;
	int y = frame / width;

	if (width == 0) EXIT_ERROR(ZERO_SIZED_TEXTURE_ERR);
	write_to[0] = x / (float)width;
	write_to[1] = 1 - (float)texObj->subSize[1] / texObj->size[1] * (y + 1);
}
