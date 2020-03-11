#include "Mesh.h"
#include "assert.h"

namespace Graphics {

	Assets::cAssetManager <cMesh> cMesh::s_manager;

	bool cMesh::Load(const std::string& i_path, cMesh*& o_mesh, std::vector<float>& i_vertices, std::vector<unsigned int>& i_indices)
	{
		auto result = true;

		cMesh* _mesh = nullptr;

		// make sure there is enough memory to allocate a model
		_mesh = new (std::nothrow) cMesh();
		if (!(result = _mesh)) {
			// Run out of memory
			// TODO: LogError: Out of memory

			return result;
		}
		else {
			_mesh->CreateMesh(&i_vertices[0], &i_indices[0], static_cast<GLuint>(i_vertices.size()), static_cast<GLuint>(i_indices.size()));
		}

		o_mesh = _mesh;

		//TODO: Loading information succeed!
		printf("Succeed! Loading mesh: %s. Vertices: %d, Indices: %d \n", i_path.c_str(), i_vertices.size(), i_indices.size());

		return result;
	}

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
			constexpr GLuint _stride = 14; // v3 (position) + v2 (texture coordinate) + v3 (vertex normal) + v3 (tangents) + v3 (biTangents)
			// the location of the pointer points to, in shader layout (location = 0)
			// size of the data that will pass in, in this case, x,y,z is 3
			// the type of the value
			// if normalize the data or not
			// stride means if skip any data, like 3 * GL_FLOAT, when we add texcood in vertices, the stride should no longer be 0
			// the offset to start the data
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(i_vertices[0]) * _stride, 0);
			// enable the attribute pointer we just created
			glEnableVertexAttribArray(0);

			// add attribute pointer for texcood
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(i_vertices[0]) * _stride, reinterpret_cast<void*>(sizeof(i_vertices[0]) * 3));
			glEnableVertexAttribArray(1);

			// add attribute pointer for normal
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(i_vertices[0]) * _stride, reinterpret_cast<void*>(sizeof(i_vertices[0]) * 5));
			glEnableVertexAttribArray(2);

			// add attribute pointer for tangents
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(i_vertices[0]) * _stride, reinterpret_cast<void*>(sizeof(i_vertices[0]) * 8));
			glEnableVertexAttribArray(3);

			// add attribute pointer for biTangents
			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(i_vertices[0]) * _stride, reinterpret_cast<void*>(sizeof(i_vertices[0]) * 11));
			glEnableVertexAttribArray(4);
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
		assert(glGetError() == GL_NO_ERROR);
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
