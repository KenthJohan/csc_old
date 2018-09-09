/*
struct Vertex_Info
{
	char * name;
	GLuint location;
	GLuint dim;
	GLenum type;
	GLboolean normalized;
	GLsizei stride;
	size_t offset;
};


void vinfo_print (struct Vertex_Info * x)
{
	printf ("%20s | ", x->name);
	printf ("%4i | ", (int)x->location);
	printf ("%4i | ", (int)x->dim);
	printf ("%4i | ", (int)x->type);
	printf ("%4i | ", (int)x->normalized);
	printf ("%4i | ", (int)x->stride);
	printf ("%4i", (int)x->offset);
	printf ("\n");
	fflush (stdout);
}



void show_structure ()
{
	struct Vertex_Info v;
	v.name = "Position";
	v.location = 0;
	v.dim = 4;
	v.type = GL_FLOAT;
	v.normalized = GL_FALSE;
	v.stride = sizeof (float) * 4;
	v.offset = 0;
	vinfo_print (&v);
}
*/
