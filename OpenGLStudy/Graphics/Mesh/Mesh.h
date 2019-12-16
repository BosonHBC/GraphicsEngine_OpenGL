#pragma once
#include "GL/glew.h"

class Mesh
{
public:
	Mesh();
	~Mesh();

	void CreateMesh(GLfloat* i_vertices, GLuint* i_indices, GLuint i_numOfVertices,GLuint i_numOfIndices);
	void Render();
	void CleanUp();

private:
	GLuint m_vao, m_vbo, m_ibo;
	GLuint m_indexCount;
};