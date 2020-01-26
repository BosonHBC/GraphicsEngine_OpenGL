#pragma once

#include <vector>

struct aiScene;
struct aiNode;
struct aiMesh;
namespace Graphics {

	/** Forward declaration*/
	class cMesh;
	class cTexture;
	/** Model stores information of meshes group and related textures*/
	class cModel
	{
	public:
		/** Constructor and destructor*/
		cModel() {};
		~cModel() { CleanUp(); }

		/** Usage functions*/
		bool LoadModel(const char* i_path);
		void Render();
		void CleanUp();

	private:

		/** Mesh list and texture list should not be stored here,
			but for the sake of simplicity, put it here first
		*/
		std::vector<cMesh*> m_meshList;
		std::vector<cTexture*> m_textureList;
		std::vector<unsigned int> m_mesh_to_texture;

		/** private helper functions*/
		void LoadNode(const aiNode* i_node, const aiScene* i_scene);
		void LoadMesh(const aiMesh* i_mesh, const aiScene* i_scene);
		void LoadMaterials(const aiScene* i_scene);
	};


}
