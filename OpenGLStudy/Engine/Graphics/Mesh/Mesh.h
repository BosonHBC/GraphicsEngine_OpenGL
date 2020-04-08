#pragma once
#include "GL/glew.h"
#include "Engine/Assets/AssetManager.h"
#include "Engine/Assets/Handle.h"
namespace Graphics {
	enum EMeshType : uint8_t
	{
		EMT_Mesh = 0,
		EMT_Point = 1,
		EMT_Line = 2,
	};
	// cMesh represent a triangular mesh which contains mesh data, handle drawing the mesh
	class cMesh
	{
	public:
		//--------------------------
		// Asset management
		using HANDLE = Assets::cHandle<cMesh>;
		static Assets::cAssetManager < cMesh > s_manager;
		static bool Load(const std::string& i_path, cMesh*& o_mesh, const EMeshType& i_meshType,std::vector<float>& i_vertices, std::vector<unsigned int>& i_indices);
		//--------------------------

		// static draw
		//--------------------------

		/** Constructors and destructor */
		cMesh();
		~cMesh();

		/** Initializations and clean up*/
		void CreateMesh(GLfloat* i_vertices, GLuint* i_indices, GLuint i_numOfVertices, GLuint i_numOfIndicesm);
		void CreatePoint(GLfloat* i_vertices, GLuint i_numOfVertices);
		void CleanUp();
		void Render(GLenum i_drawMode = GL_TRIANGLES);

	private:
		/** private variables*/
		GLuint m_vao = static_cast<GLuint>(-1), m_vbo = static_cast<GLuint>(-1), m_ibo = static_cast<GLuint>(-1);
		GLuint m_indexCount = 0;
		EMeshType m_meshType = EMT_Mesh;
	};
}
