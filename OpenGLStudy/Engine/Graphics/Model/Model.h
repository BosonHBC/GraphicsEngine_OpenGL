#pragma once
#include <vector>
#include "Engine/Graphics/Material/Material.h"
#include "Engine/Graphics/Mesh/Mesh.h"
#include "Engine/Assets/AssetManager.h"
#include "Engine/Assets/Handle.h"
#include "Cores/Utility/ISelectable.h"
struct aiScene;
struct aiNode;
struct aiMesh;
namespace Graphics {
	/** Model stores information of meshes group and related textures*/
	class cModel : public ISelectable
	{
	public:

		cModel() { }
		cModel(const std::string& i_path);
		cModel(const cModel& i_other) : ISelectable(i_other), m_meshList(i_other.m_meshList), m_materialList(i_other.m_materialList) { }
		cModel& operator = (const cModel& i_rhs) { ISelectable::operator=(i_rhs); m_meshList = i_rhs.m_meshList; m_materialList = i_rhs.m_materialList; return *this; }

		//--------------------------
		/** Destructor*/
		~cModel() { }

		/** Usage functions*/
		void UpdateUniformVariables(GLuint i_programID);
		void Render(GLenum i_drawMode = GL_TRIANGLES) const;
		// This rendering only draw elements without using material data
		// Usually is used for shadow map
		void RenderWithoutMaterial(GLenum i_drawMode = GL_TRIANGLES) const;
		void CleanUp();

		bool IntersectWithSphere(const cSphere& i_transformedSphere);
		/** Getters */
		cMaterial::HANDLE GetMaterialAt(GLuint i_idx = 0);
	private:

		/** Mesh list and texture list should not be stored here,
			but for the sake of simplicity, put it here first
		*/
		std::vector<cMesh::HANDLE> m_meshList;
		std::vector<cMaterial::HANDLE> m_materialList;

		/** private helper functions*/
		bool LoadFileFromLua(const char* i_path, std::string& o_modelPath, std::string& o_materialPath);
		void LoadNode(const char* i_path, const aiNode* i_node, const aiScene* i_scene);
		void LoadMesh(const char* i_path, const aiMesh* i_mesh, const aiScene* i_scene);
		void LoadMaterials(const aiScene* i_scene, const char* i_matName);

		// actual loading function
		bool LoadModel(const char* i_path);

	};




}
