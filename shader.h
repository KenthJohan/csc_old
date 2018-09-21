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











void program_delete (GLuint program)
{
	GLuint shaders [10];
	GLsizei count = 10;
	glGetAttachedShaders (program, count, &count, shaders);
	TRACE_F ("glGetAttachedShaders %i", (int)count);
	//The shader objects are needed only for linking the program.
	//We can delete them after the program is linked.
	for (size_t i = 0; i < count; i = i + 1)
	{
		glDetachShader (program, shaders [i]);
		glDeleteShader (shaders [i]);
		shaders [i] = 0;
	}
}







GLint program_attach1 
(GLuint program, char const * filename, GLenum kind, size_t info_n, char * info)
{
	if (kind != 0) {}
	else if (strstr (filename, ".glvs"))
	{
		kind = GL_VERTEX_SHADER;
	}
	else if (strstr (filename, ".glfs"))
	{
		kind = GL_FRAGMENT_SHADER;
	}
	ASSERT (kind != 0);
	
	GLuint shader = glCreateShader (kind);
	ASSERT (shader != 0);
	
	//Set the shader source code:
	char * buffer = app_malloc_file (filename);
	glShaderSource (shader, 1, (const GLchar **) &buffer, NULL);
	free (buffer);
	
	glCompileShader (shader);
	GLint status;
	glGetShaderiv (shader, GL_COMPILE_STATUS, &status);
	
	//char info [512];
	//size_t info_n = 512;
	//Compiler error when status = GL_FALSE:
	if (status == GL_FALSE)
	{
		//If user provide info char buffer:
		if (info)
		{
			//Put the error message in (info)
			glGetShaderInfoLog (shader, info_n, NULL, info);
			info [info_n - 1] = '\0';
			//TRACE_F ("GL_COMPILE_STATUS %s", info);
		}
		return status;
	}
	//Compule success:
	//ASSERT_F (status == GL_TRUE, "Shader (%i) compilation failed: %s", shader, info);
	glAttachShader (program, shader);
	return status;
}


GLuint program_from_filenames (char const * filenames, size_t info_n, char * info)
{
	GLuint program = glCreateProgram ();
	ASSERT (program > 0);
	struct str_ab s;
	s.a = filenames;
	//s.a = "src/project_grpagica/shader.glvs;src/project_graphica/shader.glfs";
	char token [100];
	while (1)
	{
		str_cpytok (100, token, &s, ";");
		TRACE_F ("Token: %s", token);
		GLuint status = program_attach1 (program, token, 0, info_n, info);
		if (status == GL_FALSE) 
		{
			program_delete (program);
			return 0;
		}
		if (s.b == NULL) {break;}
	}
	
	glLinkProgram (program);
	
	GLint status;
	glGetProgramiv (program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		TRACE ("");
		program = 0;
	}
	
	GL_CHECK_ERROR;
		
	return program;
}

