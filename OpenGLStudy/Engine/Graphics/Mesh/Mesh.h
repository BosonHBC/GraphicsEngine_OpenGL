#pragma once
#include "GL/glew.h"
#include "Engine/Assets/AssetManager.h"
#include "Engine/Assets/Handle.h"
namespace Graphics {
	enum EMeshType : uint8_t
	{
		EMT_Mesh = 0,
		EMT_Point = 1,
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
		void Render();

	private:
		/** private variables*/
		GLuint m_vao, m_vbo, m_ibo;
		GLuint m_indexCount;
		EMeshType m_meshType;
	};
}
