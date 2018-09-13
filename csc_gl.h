#pragma once



struct Vertex_Format
{
	GLuint vao;
	//The generic vertex attribute array being described.
	GLuint location;
	//The number of values per vertex that are stored in the array.
	GLint size;
	//The type of the data stored in the array.
	GLenum type;
	//Specifies whether fixed-point data values should be normalized (GL_TRUE) or converted directly as fixed-point values (GL_FALSE) when they are accessed. This parameter is ignored if type is GL_FIXED.
	GLboolean normalized;
	//The distance between elements within the buffer.
	GLuint offset;
	
	//GL_ARRAY_BUFFER
	//GL_ELEMENT_ARRAY_BUFFER
	GLenum target;
	GLuint buffer;
};

struct Vertex_Format Vertex_Format 
(GLuint location, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
{
	struct Vertex_Format f;
	f.location = location;
	f.size = size;
	f.type = type;
	f.normalized = normalized;
	f.offset = offset;
	return f;
}


struct VAO
{
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
	struct Vertex_Format * format;
};


