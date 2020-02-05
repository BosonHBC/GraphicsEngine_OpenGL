#include "Model/Model.h"

#include "Mesh/Mesh.h"
#include "Texture/Texture.h"
#include "Engine/Constants/Constants.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

// Static variable definition
Assets::cAssetManager<Graphics::cModel> Graphics::cModel::s_manager;

namespace Graphics {

	bool cModel::LoadModel(const char* i_path)
	{
		Assimp::Importer _importer;
		auto _scene = _importer.ReadFile(i_path, 
			aiProcess_Triangulate 
			| aiProcess_FlipUVs 
			| aiProcess_GenSmoothNormals 
			| aiProcess_JoinIdenticalVertices
		);

		if (!_scene) {
			printf("Fail to load model[%s] file:%s", i_path, _importer.GetErrorString());
			return false;
		}

		// Load nodes
		LoadNode(_scene->mRootNode, _scene);
		//  Load materials
		LoadMaterials(_scene);

		return true;
	}

	bool cModel::Load(const std::string& i_path, cModel*& o_model)
	{
		auto result = true;

		cModel* _model = nullptr;

		// make sure there is enough memory to allocate a model
		_model = new (std::nothrow) cModel();
		if (!_model) {
			// Run out of memory
			// TODO: LogError: Out of memory
			result = false;
			return result;
		}
		else {
			if (!(result = _model->LoadModel(i_path.c_str()))) {
				// TODO: LogError: fail to log
				return result;
			}
		}

		o_model = _model;
		return result;
	}

	void cModel::Render()
	{

		for (size_t i = 0; i < m_meshList.size(); ++i)
		{
			auto _matIndex = m_mesh_to_texture[i];

			// if _maxIndex is in range and the texture is not a nullptr
			if (_matIndex < m_textureList.size() && m_textureList[_matIndex]) {
				m_textureList[_matIndex]->UseTexture(GL_TEXTURE0);
			}

			m_meshList[i]->Render();
		}
	}

	void cModel::CleanUp()
	{
		for (size_t i = 0; i < m_textureList.size(); ++i)
		{
			if (m_textureList[i]) {
				delete m_textureList[i];
				m_textureList[i] = nullptr;
			}
		}
		m_textureList.clear();
		m_textureList.~vector();

		for (size_t i = 0; i < m_meshList.size(); ++i)
		{
			if (m_meshList[i]) {
				delete m_meshList[i];
				m_meshList[i] = nullptr;
			}
		}
		m_meshList.clear();
		m_meshList.~vector();
	
		m_mesh_to_texture.clear();
		m_mesh_to_texture.~vector();
	}

	void cModel::LoadNode(const aiNode* i_node, const aiScene* i_scene)
	{
		// Load parent meshes
		for (size_t i = 0; i < i_node->mNumMeshes; ++i)
		{
			LoadMesh(i_scene->mMeshes[i_node->mMeshes[i]], i_scene);
		}
		// go through child nodes
		for (size_t i = 0; i < i_node->mNumChildren; ++i)
		{
			LoadNode(i_node->mChildren[i], i_scene);
		}
	}

	void cModel::LoadMesh(const aiMesh* i_mesh, const aiScene* i_scene)
	{
		std::vector<float> _vertices;
		std::vector<unsigned int> _indices;

		for (size_t i = 0; i < i_mesh->mNumVertices; ++i)
		{
			//1. Insert vertices at the end of the data
			_vertices.insert(_vertices.end(), { i_mesh->mVertices[i].x, i_mesh->mVertices[i].y, i_mesh->mVertices[i].z });
			
			//2. Insert texture coordinate data after vertices
			if (i_mesh->mTextureCoords[0]) {
				// if first coordinate exists
				_vertices.insert(_vertices.end(), { i_mesh->mTextureCoords[0][i].x, i_mesh->mTextureCoords[0][i].y });
			}
			else {
				// there is no texture coordinate, give it default one
				_vertices.insert(_vertices.end(), {0.0f, 0.0f });
			}

			// 3. Insert normals
			_vertices.insert(_vertices.end(), { i_mesh->mNormals[i].x, i_mesh->mNormals[i].y, i_mesh->mNormals[i].z });

		}

		for (size_t i = 0; i < i_mesh->mNumFaces; ++i)
		{
			aiFace* _face = &i_mesh->mFaces[i];
			for (size_t j = 0; j < _face->mNumIndices; ++j)
			{
				_indices.push_back(_face->mIndices[j]);
			}
		}

		cMesh* _newMesh = new cMesh();
		_newMesh->CreateMesh(&_vertices[0], &_indices[0], static_cast<GLuint>(_vertices.size()), static_cast<GLuint>(_indices.size()));

		// Stored new mesh to the list and store its index to material
		m_meshList.push_back(_newMesh);
		m_mesh_to_texture.push_back(i_mesh->mMaterialIndex);


	}

	void cModel::LoadMaterials(const aiScene* i_scene)
	{
		const size_t _numOfMaterials = i_scene->mNumMaterials;
		m_textureList.resize(_numOfMaterials);

		for (size_t i = 0; i < _numOfMaterials; ++i)
		{
			aiMaterial* _material = i_scene->mMaterials[i];
			
			m_textureList[i] = nullptr;

			if (_material->GetTextureCount(aiTextureType_DIFFUSE)) {
				aiString _path;
				if (_material->GetTexture(aiTextureType_DIFFUSE, 0, &_path) == AI_SUCCESS) {
					auto _idx = std::string(_path.data).rfind("\\");
					std::string _filename = std::string(_path.data).substr(_idx + 1);

					std::string _texPath = std::string("Contents/textures/") + _filename;

					m_textureList[i] = new cTexture(_texPath.c_str());

					if (!m_textureList[i]->LoadTexture()) {
						printf("Fail to load texture[%s] file", _texPath.c_str());
						delete m_textureList[i];
						m_textureList[i] = nullptr;
					}
				}
			}
			// Fail to load texture, set it to default texture
			if (!m_textureList[i]) {
				m_textureList[i] = new cTexture(Constants::CONST_PATH_DEFAULT_TEXTURE);
				m_textureList[i]->LoadTexture();
			}
		}


	}

}
