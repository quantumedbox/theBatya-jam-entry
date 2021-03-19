#pragma once

#include <stdio.h>

#define CONTACT_ME
#ifdef RELEASE
#define CONTACT_ME printf("please, contact me at ... and tell about the problem!\n");
#endif

typedef enum error {
	GLFW_INIT_ERR,
	GLEW_INIT_ERR,
	WINDOW_CREATION_ERR,
	LOADING_SHADER_ERR,
	COMPILING_SHADER_ERR,
	LINKING_SHADER_ERR,
	ITER_ACCESS_OUTOFBOUNDS_ERR,
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
};

typedef enum warning {
	CREATED_UNKNOWN_SCENEOBJ_TYPE,
}
Warning_T;

char* WARNINGTEXT[] = {
	"created screen object with unknown type\n",
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
