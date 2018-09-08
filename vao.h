#pragma once

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glcorearb.h>


//void glVertexArrayAttribFormat 
//GLuint vaobj 
//GLuint attribindex 
//GLint size 
//GLenum type 
//GLboolean normalized 
//GLuint relativeoffset

//void glVertexArrayVertexBuffer 
//GLuint vaobj
//GLuint bindingindex
//GLuint buffer
//GLintptr offset
//GLsizei stride

//void glVertexArrayAttribBinding
//GLuint vaobj
//GLuint attribindex
//GLuint bindingindex

//void glEnableVertexArrayAttrib
//GLuint vaobj
//GLuint index

//void glDisableVertexArrayAttrib
//GLuint vaobj
//GLuint index




struct Vertex_Format
{
	//* glVertexArrayAttribFormat
	//* glEnableVertexArrayAttrib
	//* glDisableVertexArrayAttrib
	//* glVertexArrayVertexBuffer
	//* glVertexArrayAttribBinding
	//Specifies the name of the vertex array object for glVertexArrayAttrib{I, L}Format functions.
	GLuint vao;
	
	//* glVertexArrayAttribFormat
	//* glVertexArrayAttribBinding
	//* glEnableVertexArrayAttrib
	//* glDisableVertexArrayAttrib
	//The generic vertex attribute array being described.
	//This index is used by shaders to refer to the attribute using layout(location=attribute_index).
	GLuint attribute_index;
	
	//* glVertexArrayAttribFormat
	//The number of values per vertex that are stored in the array.
	GLenum size;
	
	//* glVertexArrayAttribFormat
	//The type of the data stored in the array.
	GLenum type;
	
	//* glVertexArrayAttribFormat
	//Specifies whether fixed-point data values should be normalized (GL_TRUE) or 
	//converted directly as fixed-point values (GL_FALSE) when they are accessed. 
	//This parameter is ignored if type is GL_FIXED. 
	GLboolean normalized;
	
	//* glVertexArrayAttribFormat
	//The distance between elements within the buffer.
	GLuint relative_offset;

	//* glEnableVertexArrayAttrib
	//* glDisableVertexArrayAttrib
	//Enable or disable a generic vertex attribute array
	GLboolean enabled;
	
	//* glVertexArrayAttribBinding
	//* glVertexArrayVertexBuffer
	//The index of the vertex buffer binding point to which to bind the buffer.
	//The index of the vertex buffer binding with which to associate the generic vertex attribute. 
	GLuint binding_index;
	
	//* glVertexArrayVertexBuffer
	//The name of a buffer to bind to the vertex buffer binding point.
	GLuint vbo;
	
	//* glVertexArrayVertexBuffer
	//The offset of the first element of the buffer.
	GLintptr offset;
	
	//* glVertexArrayVertexBuffer
	//The distance between elements within the buffer.
	GLsizei stride;
};


void vf_update (struct Vertex_Format * f)
{
	//Enable or disable a generic vertex attribute array
	if (f->enabled) {glEnableVertexArrayAttrib  (f->vao, f->attribute_index);}
	else            {glDisableVertexArrayAttrib (f->vao, f->attribute_index);}
	
	//Specify the organization of vertex arrays
	glVertexArrayAttribFormat 
	(
		f->vao, 
		f->attribute_index, 
		f->size, 
		f->type, 
		f->normalized, 
		f->relative_offset
	);
	
	//Associate a vertex attribute and a vertex buffer binding for a vertex array object
	glVertexArrayAttribBinding (f->vao, f->attribute_index, f->binding_index);
	
	//Bind a buffer to a vertex buffer bind point
	glVertexArrayVertexBuffer (f->vao, f->binding_index, f->vbo, f->offset, f->stride);
	
	//glNamedBufferStorage (f->vao, 0, f->vbo, f->offset, f->);
}
