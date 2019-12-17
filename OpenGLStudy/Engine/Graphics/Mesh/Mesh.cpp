#include "Mesh.h"

namespace Graphics {
	cMesh::cMesh()
	{
		m_vao = 0;
		m_vbo = 0;
		m_ibo = 0;
		m_indexCount = 0;
	}

	cMesh::~cMesh()
	{
		CleanUp();
	}

	void cMesh::CreateMesh(GLfloat* i_vertices, GLuint* i_indices, GLuint i_numOfVertices, GLuint i_numOfIndices)
	{
		m_indexCount = i_numOfIndices;

		// VAO
		{
			// generate a VAO
			glGenVertexArrays(1, &m_vao);
			// opengl will use this VAO
			glBindVertexArray(m_vao);

		}

		// IBO
		{
			glGenBuffers(1, &m_ibo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexCount * sizeof(i_indices[0]), i_indices, GL_STATIC_DRAW);
		}

		// VBO
		{
			// generate a VBO
			glGenBuffers(1, &m_vbo);
			// bind this VBO to the VAO just created
			glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
			// connect the buffer data(the vertices that just created) to gl array buffer for this vbo
			glBufferData(GL_ARRAY_BUFFER, i_numOfVertices * sizeof(i_vertices[0]), i_vertices, GL_STATIC_DRAW); // static draw, means that this vertices will not change
		}

		// Attribute pointer
		{
			// the location of the pointer points to, in shader layout (location = 0)
			// size of the data that will pass in, in this case, x,y,z is 3
			// the type of the value
			// if normalize the data or not
			// stride means if skip any data, like 3 * GL_FLOAT, when we add texcood in vertices, the stride should no longer be 0
			// the offset to start the data
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(i_vertices[0]) * 5, 0);
			// enable the attribute pointer we just created
			glEnableVertexAttribArray(0);

			// add attribute pointer for texcood
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(i_vertices[0]) * 5, reinterpret_cast<void*>(sizeof(i_vertices[0]) * 3));
			glEnableVertexAttribArray(1);
		}

		// unbind VBO
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		//unbind IBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		// unbind VAO
		glBindVertexArray(0);
	}

	void cMesh::Render()
	{
		// bind VAO
		glBindVertexArray(m_vao);

		//bind IBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
		// Index draw
		glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);

		// Vertex draw
		//glDrawArrays(GL_TRIANGLES, 0, 3);

		// clear IBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		// clear VAO
		glBindVertexArray(0);
	}

	void cMesh::CleanUp()
	{
		if (m_vbo != 0) {
			glDeleteBuffers(1, &m_vbo);
			m_vbo = 0;
		}
		if (m_ibo != 0) {
			glDeleteBuffers(1, &m_ibo);
			m_ibo = 0;
		}
		if (m_vao != 0) {
			glDeleteVertexArrays(1, &m_vao);
			m_vao = 0;
		}
		m_indexCount = 0;
	}

}
