#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "errors.h"

char* loadShaderFromFile(char *dir)
{
	FILE *p_file = fopen(dir, "r");
	if (!p_file) {
		FILE_ERROR(LOADING_SHADER_ERR, dir);
	}
	// Cheking the size of the file to know how much memory to allocate
	fseek(p_file, 0L, SEEK_END);
	char *code = (char*) malloc(ftell(p_file) + 1);
	rewind(p_file);

	char read_buff = 0;
	char *p_code = code;
	while ((read_buff = getc(p_file)) != EOF) {
		*(p_code++) = read_buff;
	}
	*p_code = '\0';

	fclose(p_file);
	return code;
}

void getOpenGLlog(GLuint ptr, GLenum status_type, char* log)
{
	int log_size;
	glGetShaderiv(ptr, status_type, &log_size);
	log = (char*)malloc(log_size);
	glGetShaderInfoLog(ptr, log_size, NULL, log);
}

void compileShader(GLuint *p_shader, const char *shader_code, GLenum shader_type)
{
	*p_shader = glCreateShader(shader_type);
	glShaderSource(*p_shader, 1, &shader_code, NULL);
	glCompileShader(*p_shader);

	GLint is_successful;
	glGetShaderiv(*p_shader, GL_COMPILE_STATUS, &is_successful);
	if (!is_successful)
	{
		char* log = '\0';
		getOpenGLlog(*p_shader, GL_COMPILE_STATUS, log);
		LOG_ERROR(COMPILING_SHADER_ERR, log);
	}
}

void linkShaderProgram(GLuint program, unsigned char n, ...)
{
	va_list(args);
	va_start(args, n);
	while (n--) {
		glAttachShader(program, va_arg(args, GLuint));
	}
	glLinkProgram(program);

	GLint is_successful;
	glGetProgramiv(program, GL_LINK_STATUS, &is_successful);
	if (!is_successful) {
		char* log = '\0';
		getOpenGLlog(program, GL_LINK_STATUS, log);
		LOG_ERROR(LINKING_SHADER_ERR, log);
	}
}

GLuint newRenderProgram(char* vertexDir, char* fragmentDir)
{
	GLuint vertexShader, fragmentShader;
	char *vertexShaderCode = loadShaderFromFile(vertexDir);
	char *fragmentShaderCode = loadShaderFromFile(fragmentDir);

	compileShader(&vertexShader, vertexShaderCode, GL_VERTEX_SHADER);
	compileShader(&fragmentShader, fragmentShaderCode, GL_FRAGMENT_SHADER);

	GLuint renderProgram = glCreateProgram();
	linkShaderProgram(renderProgram, 2, vertexShader, fragmentShader);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	free(vertexShaderCode);
	free(fragmentShaderCode);

	return renderProgram;
}
