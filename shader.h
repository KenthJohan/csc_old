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
//Returns a compiled shader on success.
//Returns a non compiled shader on error.
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


struct GL_Shader_State
{
	GLint type;
	GLint delete_status;
	GLint compile_status;
	GLint infolog_length;
	GLint source_length;
};


void gl_shader_get_state (GLuint shader, struct GL_Shader_State * s)
{
	glGetShaderiv (shader, GL_SHADER_TYPE, &s->type);
	glGetShaderiv (shader, GL_DELETE_STATUS, &s->delete_status);
	glGetShaderiv (shader, GL_COMPILE_STATUS, &s->compile_status);
	glGetShaderiv (shader, GL_INFO_LOG_LENGTH, &s->infolog_length);
	glGetShaderiv (shader, GL_SHADER_SOURCE_LENGTH, &s->source_length);
}


//Show extra information about a GL program.
void gl_program_debug (GLuint program)
{
	GLsizei count = 10;
	GLuint shaders [10];
	glGetAttachedShaders (program, count, &count, shaders);
	fprintf (stderr, "Program (%i)\n", (int) program);
	fprintf (stderr, "\u251C%10s %20s %10s %10s %10s %10s\n", "OBJ", "TYPE", "DELETE", "COMPILE", "LOGLEN", "SRCLEN");
	for (GLsizei i = 0; i < count; ++ i)
	{
		struct GL_Shader_State s;
		GLuint o = shaders [i];
		gl_shader_get_state (o, &s);
		char const * typestr = gl_str_shader_type (s.type);
		char const * delstr = gl_str_boolean (s.delete_status);
		char const * compstr = gl_str_boolean (s.compile_status);
		fprintf (stderr, "\u251C");
		fprintf (stderr, "%10i ", (int) o);
		fprintf (stderr, "%20s " , typestr);
		fprintf (stderr, "%10s ", delstr);
		fprintf (stderr, "%10s ", compstr);
		fprintf (stderr, "%10i ", (int) s.infolog_length);
		fprintf (stderr, "%10i ", (int) s.source_length);
		fprintf (stderr, "\n");
	}
	fflush (stderr);
}

