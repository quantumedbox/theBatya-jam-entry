#pragma once

#include "camera.h"
#include "iter.h"
#include "input.h"

typedef unsigned int uint;

typedef uint uvec2[2];
typedef uint uvec3[3];


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

// Stores texture sheet data for sprites
typedef struct
{
	GLuint texture_ptr;	// Pointer to texture buffer
	uvec2 size;			// Width / Height in texels
	uvec2 subSize;		// Width / Height for each subtexture
}
TextureObj;

#define NO_RENDER_PROGRAM 0
typedef struct
{
	GLuint renderProgram;
	Geometry* geometry;
	TextureObj* textureObj;

	float scale;

	float frame;
	uint frameCount;
	float animationSpeed;
}
RenderObj;

typedef struct {
	uvec3 clearColor;	// Base color of the screen
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
	Iter* objs;			// All game objects that should be processed
	vec3 position;		// Origin point for all object within the scene
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
