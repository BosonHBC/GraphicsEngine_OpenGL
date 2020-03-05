#pragma once
#include <vector>
#include "Engine/Graphics/Material/Material.h"
#include "Engine/Graphics/Mesh/Mesh.h"
#include "Engine/Assets/AssetManager.h"
#include "Engine/Assets/Handle.h"

struct aiScene;
struct aiNode;
struct aiMesh;
namespace Graphics {
	/** Model stores information of meshes group and related textures*/
	class cModel
	{
	public:
		//--------------------------
		// Asset management
		using HANDLE = Assets::cHandle<cModel>;
		static Assets::cAssetManager < cModel > s_manager;
		static bool Load(const std::string& i_path, cModel*& o_model);
		//--------------------------

		/** Destructor*/
		~cModel() { CleanUp(); }

		/** Usage functions*/
		void UpdateUniformVariables(GLuint i_programID);
		void Render();
		// This rendering only draw elements without using material data
		// Usually is used for shadow map
		void RenderWithoutMaterial();
		void CleanUp();

		/** Getters */
		cMaterial* GetMaterialAt(GLuint i_idx = 0);
	private:

		/** private default constructor, that can not be used by others*/
		cModel() {
		}

		/** Mesh list and texture list should not be stored here,
			but for the sake of simplicity, put it here first
		*/
		std::vector<cMesh::HANDLE> m_meshList;
		std::vector<cMaterial*> m_materialList;

		/** private helper functions*/
		bool LoadFileFromLua(const char* i_path, std::string& o_modelPath, std::string& o_materialPath);
		void LoadNode(const char* i_path, const aiNode* i_node, const aiScene* i_scene);
		void LoadMesh(const char* i_path, const aiMesh* i_mesh, const aiScene* i_scene);
		void LoadMaterials(const aiScene* i_scene, const char* i_matName);

		// actual loading function
		bool LoadModel(const char* i_path);
	};




}
