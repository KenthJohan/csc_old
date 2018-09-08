#pragma once
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glcorearb.h>
#include "misc.h"
#include "debug_gl.h"


struct Shader
{
	GLuint handle;
	GLenum kind;
	char * filename;
};


void shader_create (struct Shader * shader)
{
	ASSERT_F 
	(
		(shader->kind == GL_VERTEX_SHADER) || 
		(shader->kind == GL_FRAGMENT_SHADER), 
		"Unsupported shader kind%s", ""
	);
	shader->handle = glCreateShader (shader->kind);
	ASSERT_F (shader != 0, "glCreateShader error%s", "");
	char * buffer = app_malloc_file (shader->filename);
	glShaderSource (shader->handle, 1, (const GLchar **) &buffer, NULL);
	free (buffer);
	glCompileShader (shader->handle);
	GLint status;
	glGetShaderiv (shader->handle, GL_COMPILE_STATUS, &status);
	char err_buf [512];
	if (status != GL_TRUE)
	{
		glGetShaderInfoLog (shader->handle, sizeof (err_buf), NULL, err_buf);
		err_buf [sizeof (err_buf) - 1] = '\0';
	}
	ASSERT_F (status == GL_TRUE, "%s compilation failed: %s", shader->filename, err_buf);
}







GLuint program_create (char const * name, struct Shader * shaders, size_t count)
{
	GLuint program;
	program = glCreateProgram ();
	if (name)
	{
		glObjectLabel (GL_PROGRAM, program, -1, name);
	}
	ASSERT_F (program != 0, "glCreateProgram error%s", "");
	for (size_t i = 0; i < count; i = i + 1)
	{
		shader_create (shaders + i);
		glAttachShader (program, shaders [i].handle);
	}
	glLinkProgram (program);
	//The shader objects are needed only for linking the program.
	//We can delete them after the program is linked.
	for (size_t i = 0; i < count; i = i + 1)
	{
		glDetachShader (program, shaders [i].handle);
		glDeleteShader (shaders [i].handle);
		shaders [i].handle = 0;
	}
	GL_CHECK_ERROR;
	return program;
}

