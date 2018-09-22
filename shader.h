#pragma once
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glcorearb.h>
#include "misc.h"
#include "debug_gl.h"


//Get the shader type depending on filename extension.
//These extensions are not standard but it makes it easy differentiate between different shader types.
GLuint gl_shader_fileext (char const * filename)
{
	if (0) {}
	else if (strstr (filename, ".glvs"))
	{
		return GL_VERTEX_SHADER;
	}
	else if (strstr (filename, ".glfs"))
	{
		return GL_FRAGMENT_SHADER;
	}
	return 0;
}



//Load a shader from a file by filename.
GLuint gl_shader_from_filename (char const * filename, GLenum kind)
{
	ASSERT (kind != 0);
	GLuint shader = glCreateShader (kind);
	ASSERT (shader != 0);
	//Set the shader source code:
	char * buffer = app_malloc_file (filename);
	glShaderSource (shader, 1, (const GLchar **) &buffer, NULL);
	free (buffer);
	glCompileShader (shader);
	return shader;
}


//Load program by filename.
//The filename must consist of multiple filenames that are refering to shader files.
//e.g. "mydir/myshader.glfs;mydir/myshader.glvs"
GLuint gl_program_from_filename (char const * filename)
{
	GLuint program = glCreateProgram ();
	ASSERT (program > 0);
	struct str_ab s;
	s.a = filename;
	char token [100];
	while (1)
	{
		str_cpytok (100, token, &s, ";");
		TRACE_F ("Token: %s", token);
		GLuint shader = gl_shader_from_filename (token, gl_shader_fileext (token));
		glAttachShader (program, shader);
		if (s.b == NULL) {break;}
	}
	glLinkProgram (program);
	GL_CHECK_ERROR;
	return program;
}


//Convert shader type to string.
char const * gl_str_shader_type (GLenum type)
{
	switch (type)
	{
		case GL_VERTEX_SHADER:
		return "GL_VERTEX_SHADER";
		case GL_FRAGMENT_SHADER:
		return "GL_FRAGMENT_SHADER";
	}
	return "";
}


//Convert GL boolean to string.
char const * gl_str_boolean (GLint value)
{
	switch (value)
	{
		case GL_TRUE:
		return "GL_TRUE";
		case GL_FALSE:
		return "GL_FALSE";
	}
	return "";
}


//Show extra information about a GL program.
void gl_program_debug (GLuint program)
{
	GLsizei count = 10;
	GLuint shaders [10];
	glGetAttachedShaders (program, count, &count, shaders);
	fprintf (stderr, "Program (%i)\n", (int) program);
	fprintf (stderr, "\u251C %10s %20s %10s %10s %10s %10s\n", "ID", "TYPE", "DELETE", "COMPILE", "LOGLEN", "SRCLEN");
	for (GLsizei i = 0; i < count; ++ i)
	{
		GLint type;
		GLint del;
		GLint comp;
		GLint loglen;
		GLint srclen;
		glGetShaderiv (shaders [i], GL_SHADER_TYPE, &type);
		glGetShaderiv (shaders [i], GL_DELETE_STATUS, &del);
		glGetShaderiv (shaders [i], GL_COMPILE_STATUS, &comp);
		glGetShaderiv (shaders [i], GL_INFO_LOG_LENGTH, &loglen);
		glGetShaderiv (shaders [i], GL_SHADER_SOURCE_LENGTH, &srclen);
		GLuint s = shaders [i];
		char const * typestr = gl_str_shader_type (type);
		char const * delstr = gl_str_boolean (del);
		char const * compstr = gl_str_boolean (comp);
		fprintf (stderr, "\u251C %10i %20s %10s %10s %10i %10i\n", (int) s, typestr, delstr, compstr, (int) loglen, (int) srclen);
	}
	fflush (stderr);
}

