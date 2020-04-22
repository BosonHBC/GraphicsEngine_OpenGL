#pragma once
#include "GL/glew.h"
#include "Engine/Assets/AssetManager.h"
#include "Engine/Assets/Handle.h"
#include "Math/Shape/Box.h"
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
		static bool Load(const std::string& i_path, cMesh*& o_mesh, const EMeshType& i_meshType,std::vector<float>& i_vertices, std::vector<unsigned int>& i_indices, const glm::vec3& i_aabbMin = glm::vec3(0), const glm::vec3& i_aabbMax = glm::vec3(0));
		//--------------------------

		// static draw
		//--------------------------

		/** Constructors and destructor */
		cMesh();
		~cMesh();

		/** Initializations and clean up*/
		void CreateMesh(GLfloat* i_vertices, GLuint* i_indices, GLuint i_numOfVertices, GLuint i_numOfIndices);
		void CreatePoint(GLfloat* i_vertices, GLuint i_numOfVertices);
		void CleanUp();
		void UpdateBufferData(GLfloat* i_verticData, GLuint i_numOfVertices );
		void UpdateIndexBufferData(GLuint* i_indices, GLuint i_numOfIndices);
		void Render(GLenum i_drawMode = GL_TRIANGLES);
		void CreateAABB(const glm::vec3& i_min, const glm::vec3& i_max);
		bool IntersectWithSphere(const cSphere& i_transformedSphere);
	private:
		/** private variables*/
		GLuint m_vao = static_cast<GLuint>(-1), m_vbo = static_cast<GLuint>(-1), m_ibo = static_cast<GLuint>(-1);
		GLuint m_indexCount = 0;
		EMeshType m_meshType = EMT_Mesh;
		cAABB m_aabb;
	};
}
