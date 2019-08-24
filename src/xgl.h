#pragma once

#include <csc_debug.h>
#include <csc_tcol.h>
#include <csc_malloc_file.h>
#include <glad/glad.h>

#define XGL_ASSERT_ERROR xgl_assert_error (__FILE__,__LINE__)
#define XGL_COLOR_SUCCESS TCOL (TCOL_NORMAL, TCOL_GREEN, TCOL_DEFAULT)
#define XGL_COLOR_ERROR TCOL (TCOL_NORMAL, TCOL_RED, TCOL_DEFAULT)
#define XGL_SHADER_NMAX 10
#define XGL_SHADER_NMAX 10


//Convert shader type to string.
char const * xgl_enum_str (GLenum type)
{
	switch (type)
	{
	case GL_VERTEX_SHADER:                  return "VERTEX_SHADER";
	case GL_FRAGMENT_SHADER:                return "FRAGMENT_SHADER";
	case GL_INVALID_OPERATION:              return "INVALID_OPERATION";
	case GL_INVALID_ENUM:                   return "INVALID_ENUM";
	case GL_INVALID_VALUE:                  return "INVALID_VALUE";
	case GL_INVALID_FRAMEBUFFER_OPERATION:  return "INVALID_FRAMEBUFFER_OPERATION";
	case GL_OUT_OF_MEMORY:                  return "OUT_OF_MEMORY";
	default: return "";
	}
	return "";
}


//Convert GL boolean to string.
char const * xgl_bool_str (GLint value)
{
	switch (value)
	{
	case GL_TRUE:return "TRUE";
	case GL_FALSE:return "FALSE";
	default: return "";
	}
	return "";
}


void xgl_assert_error (const char *file, int line)
{
	GLenum e = glGetError ();
	if (e == GL_NO_ERROR) {return;}
	while (1)
	{
		fprintf (stderr, "%s:%i %s (%i)", file, line, xgl_enum_str (e), e);
		e = glGetError ();
		if (e == GL_NO_ERROR) {break;}
	}
	exit (1);
}


void xgl_uniform1i_set (GLuint program, GLchar const * name, GLint v0)
{
	GLint location;
	location = glGetUniformLocation (program, name);
	ASSERT (location >= 0);
	glUniform1i (location, v0);
}


void xgl_program_free_shaders (GLuint program)
{
	ASSERT (glIsProgram (program));
	GLsizei i = XGL_SHADER_NMAX;
	GLuint shaders [XGL_SHADER_NMAX];
	glGetAttachedShaders (program, i, &i, shaders);
	while (i--)
	{
		glDetachShader (program, shaders [i]);
		glDeleteShader (shaders [i]);
	}
}


void xgl_attach_shaderfile (GLuint program, char const * filename, GLenum kind)
{
	ASSERT (kind != 0);
	GLuint shader = glCreateShader (kind);
	XGL_ASSERT_ERROR;
	ASSERT (shader != 0);
	//Set the shader source code:
	char * buffer = csc_malloc_file (filename);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
	glShaderSource (shader, 1, (const GLchar **) &buffer, NULL);
#pragma GCC diagnostic pop
	XGL_ASSERT_ERROR;
	free (buffer);
	glCompileShader (shader);
	glAttachShader (program, shader);
	//If a shader object to be deleted is attached to a program object, it will be flagged for deletion,
	//but it will not be deleted until it is no longer attached to any program object,
	//for any rendering context (i.e., it must be detached from wherever it was attached before it will be deleted).
	glDeleteShader (shader);
	XGL_ASSERT_ERROR;
}


enum xgl_type
{
	XGL_SHADER_TYPE,
	XGL_SHADER_DELETE_STATUS,
	XGL_SHADER_COMPILE_STATUS,
	XGL_SHADER_INFO_LOG_TEXT,
	XGL_SHADER_INFO_LOG_LENGTH,
	XGL_SHADER_SOURCE_TEXT,
	XGL_SHADER_SOURCE_LENGTH,
	XGL_PROGRAM_DELETE_STATUS,
	XGL_PROGRAM_LINK_STATUS,
	XGL_PROGRAM_VALIDATE_STATUS,
	XGL_PROGRAM_INFO_LOG_LENGTH,
	XGL_PROGRAM_ATTACHED_SHADERS,
	XGL_PROGRAM_ACTIVE_ATTRIBUTES,
	XGL_PROGRAM_ACTIVE_ATTRIBUTE_MAX_LENGTH,
	XGL_PROGRAM_ACTIVE_UNIFORMS,
	XGL_PROGRAM_ACTIVE_UNIFORM_MAX_LENGTH,
};


void xgl_print (FILE * f, GLuint obj, enum xgl_type type, int flen)
{
	char text [100];
	GLint i;
	switch (type)
	{
	case XGL_SHADER_TYPE:
		glGetShaderiv (obj, GL_SHADER_TYPE, &i);
		fprintf (f, "%*s", flen, xgl_enum_str ((GLenum)i));
		break;
	case XGL_SHADER_DELETE_STATUS:
		glGetShaderiv (obj, GL_DELETE_STATUS, &i);
		fprintf (f, "%*s", flen, xgl_bool_str (i));
		break;
	case XGL_SHADER_COMPILE_STATUS:
		glGetShaderiv (obj, GL_COMPILE_STATUS, &i);
		fprintf (f, "%*s", flen, xgl_bool_str (i));
		break;
	case XGL_SHADER_INFO_LOG_LENGTH:
		glGetShaderiv (obj, GL_INFO_LOG_LENGTH, &i);
		fprintf (f, "%*i", flen, i);
		break;
	case XGL_SHADER_INFO_LOG_TEXT:
		glGetShaderInfoLog (obj, sizeof (text), NULL, text);
		ASSERT (flen <= 100);
		fprintf (f, "%.*s", flen, text);
		break;
	case XGL_SHADER_SOURCE_LENGTH:
		glGetShaderiv (obj, GL_SHADER_SOURCE_LENGTH, &i);
		fprintf (f, "%*i", flen, i);
		break;
	case XGL_SHADER_SOURCE_TEXT:
		glGetShaderSource (obj, sizeof (text), NULL, text);
		ASSERT (flen <= 100);
		fprintf (f, "%.*s", flen, text);
		break;
	case XGL_PROGRAM_DELETE_STATUS:
		glGetProgramiv (obj, GL_DELETE_STATUS, &i);
		fprintf (f, "%*s", flen, xgl_bool_str (i));
		break;
	case XGL_PROGRAM_LINK_STATUS:
		glGetProgramiv (obj, GL_LINK_STATUS, &i);
		fprintf (f, "%*s", flen, xgl_bool_str (i));
		break;
	case XGL_PROGRAM_VALIDATE_STATUS:
		glGetProgramiv (obj, GL_VALIDATE_STATUS, &i);
		fprintf (f, "%*s", flen, xgl_bool_str (i));
		break;
	case XGL_PROGRAM_INFO_LOG_LENGTH:
		glGetProgramiv (obj, GL_INFO_LOG_LENGTH, &i);
		fprintf (f, "%*i", flen, i);
		break;
	case XGL_PROGRAM_ATTACHED_SHADERS:
		glGetProgramiv (obj, GL_ATTACHED_SHADERS, &i);
		fprintf (f, "%*i", flen, i);
		break;
	case XGL_PROGRAM_ACTIVE_ATTRIBUTES:
		glGetProgramiv (obj, GL_ACTIVE_ATTRIBUTES, &i);
		fprintf (f, "%*i", flen, i);
		break;
	case XGL_PROGRAM_ACTIVE_ATTRIBUTE_MAX_LENGTH:
		glGetProgramiv (obj, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &i);
		fprintf (f, "%*i", flen, i);
		break;
	case XGL_PROGRAM_ACTIVE_UNIFORMS:
		glGetProgramiv (obj, GL_ACTIVE_UNIFORMS, &i);
		fprintf (f, "%*i", flen, i);
		break;
	case XGL_PROGRAM_ACTIVE_UNIFORM_MAX_LENGTH:
		glGetProgramiv (obj, GL_ACTIVE_UNIFORM_MAX_LENGTH, &i);
		fprintf (f, "%*i", flen, i);
		break;
	default:
		break;
	}
}


void xgl_program_print (GLuint program)
{
	FILE * f = stderr;
	GLsizei n = XGL_SHADER_NMAX;
	GLuint shaders [XGL_SHADER_NMAX];
	glGetAttachedShaders (program, n, &n, shaders);

	fprintf (f, "+");
	fprintf (f, "%8s%10s%10s%10s%5s%10s%10s%10s\n", "PROGRAM", "LINK", "VALIDATE", "DELETE", "INFO", "ATTACHED", "ACTIVEA", "ACTIVEU");
	fprintf (f, "|");
	fprintf (f, "%8i", program);
	xgl_print (stderr, program, XGL_PROGRAM_DELETE_STATUS, 10);
	xgl_print (stderr, program, XGL_PROGRAM_LINK_STATUS, 10);
	xgl_print (stderr, program, XGL_PROGRAM_VALIDATE_STATUS, 10);
	xgl_print (stderr, program, XGL_PROGRAM_INFO_LOG_LENGTH, 5);
	xgl_print (stderr, program, XGL_PROGRAM_ATTACHED_SHADERS, 10);
	xgl_print (stderr, program, XGL_PROGRAM_ACTIVE_ATTRIBUTES, 10);
	//xgl_print (stderr, program, XGL_PROGRAM_ACTIVE_ATTRIBUTE_MAX_LENGTH, 20);
	xgl_print (stderr, program, XGL_PROGRAM_ACTIVE_UNIFORMS, 10);
	//xgl_print (stderr, program, XGL_PROGRAM_ACTIVE_UNIFORM_MAX_LENGTH, 20);
	fprintf (f, "\n");

	fprintf (f, "+");
	fprintf (f, "%7s%16s%8s%8s%8s%8s\n", "SHADER", "TYPE", "DELETE", "COMPILE", "LOGLEN", "SRCLEN");
	for (GLsizei i = 0; i < n; ++ i)
	{
		fprintf (f, "|");
		fprintf (f, "%7i", shaders [i]);
		xgl_print (stderr, shaders [i], XGL_SHADER_TYPE, 16);
		xgl_print (stderr, shaders [i], XGL_SHADER_DELETE_STATUS, 8);
		xgl_print (stderr, shaders [i], XGL_SHADER_COMPILE_STATUS, 8);
		xgl_print (stderr, shaders [i], XGL_SHADER_INFO_LOG_LENGTH, 8);
		//xgl_print (stderr, shaders [i], XGL_SHADER_SOURCE_TEXT, 20);
		xgl_print (stderr, shaders [i], XGL_SHADER_SOURCE_LENGTH, 8);
		fprintf (f, "\n");
	}


	for (GLsizei i = 0; i < n; ++ i)
	{
		fprintf (f, "SHADER %i INFO_LOG:\n", shaders [i]);
		xgl_print (stderr, shaders [i], XGL_SHADER_INFO_LOG_TEXT, 100);
		fprintf (f, "\n");
	}
	fflush (f);
}


