#pragma once

#include <stdio.h>

#define CONTACT_ME
#ifdef RELEASE
#undef CONTACT_ME
#define CONTACT_ME printf("please, report the issue at https://github.com/quantumedbox/theBatya-jam-entry!\nI will be grateful\n");
#endif

typedef enum error {
	GLFW_INIT_ERR,
	GLEW_INIT_ERR,
	WINDOW_CREATION_ERR,
	LOADING_SHADER_ERR,
	COMPILING_SHADER_ERR,
	LINKING_SHADER_ERR,
	ITER_ACCESS_OUTOFBOUNDS_ERR,
	TEXTURE_LOADING_ERR,
	ZERO_SIZED_TEXTURE_ERR,
}
Error_T;

char* ERRORTEXT[] = {
	"cannot initialize a glfw context\n",
	"cannot initialize a glew opengl extension\n",
	"cannot initialize a window context\n",
	"error occured while loading shader\n",
	"error occured while compiling shader\n",
	"error occured while linking shader\n",
	"iterator access violation, index is out of size for a given iter\n",
	"error occured while loading texture\n",
	"zero sized texture. possibly something wrong with the texture or its loading\n",
};

typedef enum warning {
	CREATED_UNKNOWN_SCENEOBJ_TYPE,
	UNKNOWN_SCENEOBJ_TYPE,
	UNKNOWN_KEY_WARN,
	SETTING_OBJ_OF_INCOMPATIBLE_TYPE_WARN,
}
Warning_T;

char* WARNINGTEXT[] = {
	"created screen object with unknown type\n",
	"encountered unknown type object in the scene and it was ignored\n",
	"unknown key type\n",
	"setting the value for incompatible obj type\n",
};

void WARNING(Warning_T warn_code)
{
	printf(WARNINGTEXT[warn_code]);
}

void PRINT_ERROR(Error_T error_code)
{
	printf(ERRORTEXT[error_code]);

	#ifdef STRICT_RUNTIME
	printf(">returning with the code 0x%x\n", error_code);
	exit(error_code);
	#endif
}

void LOG_ERROR(Error_T error_code, char* log)
{
	printf("%s\n%s\n", ERRORTEXT[error_code], log);
	free(log);

	#ifdef STRICT_RUNTIME
	printf(">returning with the code 0x%x\n", error_code);
	exit(error_code);
	#endif
}

void OPENGL_CHECK(char* description)
{
	GLenum err;
	if ((err = glGetError()) != GL_NO_ERROR) {
		printf("OPENGL ERROR :: CODE %d\n%s\n", err, description);
		#ifdef STRICT_RUNTIME
		exit(0);
		#endif
	}
}

void EXIT_ERROR(Error_T error_code)
{
	printf("RUNTIME ERROR:\n");
	printf(ERRORTEXT[error_code]);
	CONTACT_ME
	printf(">returning with the code 0x%x\n", error_code);
	exit(error_code);
}

void OPENGL_ERROR(Error_T error_code, GLenum err)
{
	printf("OPENGL ERROR:\n");
	printf(ERRORTEXT[error_code]);
	printf("--code of the error: %d\n", err);
	CONTACT_ME
	printf(">returning with the code 0x%x\n", error_code);
	exit(error_code);
}

void FILE_ERROR(Error_T error_code, char* dir)
{
	printf("FILE ERROR:\n");
	printf(ERRORTEXT[error_code]);
	printf("--cannot access file at: %s\n", dir);
	CONTACT_ME
	printf(">returning with the code 0x%x\n", error_code);
	exit(error_code);
}
