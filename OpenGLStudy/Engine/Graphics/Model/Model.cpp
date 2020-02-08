#include "Model/Model.h"

#include "Mesh/Mesh.h"
#include "Engine/Constants/Constants.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "Material/Material.h"
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
		if (!(result = _model)) {
			// Run out of memory
			// TODO: LogError: Out of memory

			return result;
		}
		else {
			if (!(result = _model->LoadModel(i_path.c_str()))) {
				// TODO: LogError: fail to log

				delete _model;

				return result;
			}
		}

		o_model = _model;

		//TODO: Loading information succeed!
		printf("Succeed! Loading model: %s. Mesh size: %d, texture size: %d\n", i_path.c_str(), o_model->m_meshList.size(), o_model->m_materialList.size());

		return result;
	}

	void cModel::UpdateUniformVariables(GLuint i_programID)
	{
		for (auto item : m_materialList)
		{
			cMaterial* _matInst = cMaterial::s_manager.Get(item);
			if (_matInst) {
				_matInst->UpdateUniformVariables(i_programID);
			}
		}
	}

	void cModel::Render()
	{

		for (size_t i = 0; i < m_meshList.size(); ++i)
		{

			auto _matIndex = m_mesh_to_material[i];

			cMaterial* _material = cMaterial::s_manager.Get(m_materialList[_matIndex]);
			// if _maxIndex is in range and the texture is not a nullptr
			if (_matIndex < m_materialList.size() && _material) {
				_material->UseMaterial();
			}

			m_meshList[i]->Render();
			
			if (_matIndex < m_materialList.size() && _material) {
				_material->CleanUpMaterialBind();
			}
		}
	}

	void cModel::CleanUp()
	{
		for (size_t i = 0; i < m_materialList.size(); ++i)
		{
			cMaterial::s_manager.Release(m_materialList[i]);
		}
		m_materialList.clear();
		m_materialList.~vector();

		for (size_t i = 0; i < m_meshList.size(); ++i)
		{
			if (m_meshList[i]) {
				delete m_meshList[i];
				m_meshList[i] = nullptr;
			}
		}
		m_meshList.clear();
		m_meshList.~vector();

		m_mesh_to_material.clear();
		m_mesh_to_material.~vector();
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
				_vertices.insert(_vertices.end(), { 0.0f, 0.0f });
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
		m_mesh_to_material.push_back(i_mesh->mMaterialIndex);


	}

	void cModel::LoadMaterials(const aiScene* i_scene)
	{
		const size_t _numOfMaterials = i_scene->mNumMaterials;
		m_materialList.resize(_numOfMaterials);

		for (size_t i = 1; i < _numOfMaterials; ++i)
		{
			aiMaterial* _material = i_scene->mMaterials[i];

			// TODO: right now, the material path is meaningless
			std::string _path =   "Invalid path_" + std::string( i_scene->mRootNode->mName.C_Str()) + std::to_string(i);
			if (!cMaterial::s_manager.Load(_path, m_materialList[i], _material)) {
				printf("Fail to load material[--] file");
				continue;
			}
		}

	}

}