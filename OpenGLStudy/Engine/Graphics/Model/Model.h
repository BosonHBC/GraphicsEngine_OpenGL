#pragma once

#include <vector>
#include "Engine/Graphics/Texture/Texture.h"

struct aiScene;
struct aiNode;
struct aiMesh;
namespace Graphics {

	/** Forward declaration*/
	class cMesh;
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
		void Render();
		void CleanUp();

	private:

		/** private default constructor, that can not be used by others*/
		cModel() {
		}

		/** Mesh list and texture list should not be stored here,
			but for the sake of simplicity, put it here first
		*/
		std::vector<cMesh*> m_meshList;
		std::vector<cTexture::HANDLE> m_textureList;
		std::vector<unsigned int> m_mesh_to_texture;

		/** private helper functions*/
		void LoadNode(const aiNode* i_node, const aiScene* i_scene);
		void LoadMesh(const aiMesh* i_mesh, const aiScene* i_scene);
		void LoadMaterials(const aiScene* i_scene);

		// actual loading function
		bool LoadModel(const char* i_path);
	};




}
