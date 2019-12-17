#pragma once
#include "GL/glew.h"

namespace Graphics {
	// cMesh represent a triangular mesh which contains mesh data, handle drawing the mesh
	class cMesh
	{
	public:
		/** Constructors and destructor */
		cMesh();
		~cMesh();

		/** Initializations and clean up*/
		void CreateMesh(GLfloat* i_vertices, GLuint* i_indices, GLuint i_numOfVertices, GLuint i_numOfIndices);
		void CleanUp();
		
		void Render();

	private:
		/** private variables*/
		GLuint m_vao, m_vbo, m_ibo;
		GLuint m_indexCount;
	};
}
