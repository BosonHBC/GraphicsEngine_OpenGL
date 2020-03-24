#include "Engine/Graphics/Model/Model.h"
#include "Engine/Graphics/Material/Material.h"

#include "Engine/Constants/Constants.h"

#include "Externals/ASSIMP_N/include/assimp/Importer.hpp"
#include "Externals/ASSIMP_N/include/assimp/scene.h"
#include "Externals/ASSIMP_N/include/assimp/postprocess.h"


#include "Assets/LoadTableFromLuaFile.h"
#include "Assets/PathProcessor.h"
// Static variable definition
Assets::cAssetManager<Graphics::cModel> Graphics::cModel::s_manager;

namespace Graphics {

	bool cModel::LoadModel(const char* i_path)
	{
		// Load model files from lua
		std::string _modelPath, _materialPath;
		// load model data from LUA files
		if (!LoadFileFromLua(i_path, _modelPath, _materialPath)) {
			printf("Fail to load model[%s] from LUA.\n", i_path);
			return false;
		}


		Assimp::Importer _importer;
		auto _scene = _importer.ReadFile(_modelPath.insert(0, Constants::CONST_PATH_MODLE_ROOT),
			aiProcess_Triangulate
			| aiProcess_FlipUVs
			| aiProcess_GenSmoothNormals
			| aiProcess_JoinIdenticalVertices
			| aiProcess_CalcTangentSpace // this operation will ask assimp calculate the tangent space for us automatically
		);

		if (!_scene) {
			printf("Fail to load model[%s] file:%s", i_path, _importer.GetErrorString());
			return false;
		}

		// Load nodes
		LoadNode(_modelPath.c_str(), _scene->mRootNode, _scene);
		//  Load materials
		LoadMaterials(_scene, _materialPath.c_str());

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
		//printf("Succeed! Loading model: %s. Mesh size: %d, texture size: %d\n", i_path.c_str(), o_model->m_meshList.size(), o_model->m_materialList.size());

		return result;
	}

	void cModel::UpdateUniformVariables(GLuint i_programID)
	{
		for (auto item : m_materialList)
		{
			cMaterial* _matInst = item;
			if (_matInst) {
				_matInst->UpdateUniformVariables(i_programID);
			}
		}
	}

	void cModel::Render()
	{

		for (size_t i = 0; i < m_meshList.size(); ++i)
		{
			cMaterial* _material = m_materialList[0];
			// if _maxIndex is in range and the texture is not a nullptr
			if (_material) {
				_material->UseMaterial();
			}
			
			auto _mesh = cMesh::s_manager.Get(m_meshList[i]);
			if (_mesh)
				_mesh->Render();

			if (_material) {
				_material->CleanUpMaterialBind();
			}
		}
	}

	void cModel::RenderWithoutMaterial()
	{
		for (size_t i = 0; i < m_meshList.size(); ++i)
		{
			auto _mesh = cMesh::s_manager.Get(m_meshList[i]);
			if (_mesh)
				_mesh->Render();
		}
	}

	void cModel::CleanUp()
	{
		for (size_t i = 0; i < m_materialList.size(); ++i)
		{
			safe_delete(m_materialList[i]);
		}
		m_materialList.clear();
		m_materialList.~vector();

		for (size_t i = 0; i < m_meshList.size(); ++i)
		{
			cMesh::s_manager.Release(m_meshList[i]);
		}
		m_meshList.clear();
		m_meshList.~vector();
	}

	Graphics::cMaterial* cModel::GetMaterialAt(GLuint i_idx /*= 0*/)
	{
		if (i_idx < m_materialList.size()) {
			return m_materialList[i_idx];
		}
	}

	bool cModel::LoadFileFromLua(const char* i_path, std::string& o_modelPath, std::string& o_materialPath)
	{
		auto result = true;
		lua_State* luaState = nullptr;
		//------------------------------
		// Initialize Lua
		//------------------------------
		if (!(result = Assets::InitializeLUA(i_path, luaState))) {
			Assets::ReleaseLUA(luaState);
			return result;
		}
		//------------------------------
		// Load data
		//------------------------------
		{
			// o_modelPath
			{
				constexpr auto* const _key = "ModelPath";
				if (!(result = Assets::Lua_LoadString(luaState, _key, o_modelPath))) {
					printf("LUA error: fail to load key[%s]", _key);
					return result;
				}
			}
			// o_materialPath
			{
				constexpr auto* const _key = "MaterialPath";
				if (!(result = Assets::Lua_LoadString(luaState, _key, o_materialPath))) {
					printf("LUA error: fail to load key[%s]", _key);
					return result;
				}
			}
		}
		//------------------------------
		// Release Lua
		//------------------------------
		result = Assets::ReleaseLUA(luaState);

		return result;

	}

	void cModel::LoadNode(const char* i_path, const aiNode* i_node, const aiScene* i_scene)
	{
		// Load parent meshes
		for (size_t i = 0; i < i_node->mNumMeshes; ++i)
		{
			LoadMesh(i_path, i_scene->mMeshes[i_node->mMeshes[i]], i_scene);
		}
		// go through child nodes
		for (size_t i = 0; i < i_node->mNumChildren; ++i)
		{
			LoadNode(i_path, i_node->mChildren[i], i_scene);
		}
	}

	void cModel::LoadMesh(const char* i_path, const aiMesh* i_mesh, const aiScene* i_scene)
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

			// 4. Insert Tangents
			_vertices.insert(_vertices.end(), { i_mesh->mTangents[i].x, i_mesh->mTangents[i].y, i_mesh->mTangents[i].z });

			// 5. Insert biTangents
			_vertices.insert(_vertices.end(), { i_mesh->mBitangents[i].x, i_mesh->mBitangents[i].y, i_mesh->mBitangents[i].z });
		}

		for (size_t i = 0; i < i_mesh->mNumFaces; ++i)
		{
			aiFace* _face = &i_mesh->mFaces[i];
			for (size_t j = 0; j < _face->mNumIndices; ++j)
			{
				_indices.push_back(_face->mIndices[j]);
			}
		}
		cMesh::HANDLE _newMeshHandle;
		std::string meshKey = std::string(i_path).append("_" + std::to_string(m_meshList.size()));
		if (cMesh::s_manager.Load(meshKey, _newMeshHandle, EMT_Mesh, _vertices, _indices))
		{
			// Stored new mesh to the list and store its index to material
			m_meshList.push_back(_newMeshHandle);
		}
		else
		{
			printf("Loading Model fail: Fail to create mesh%s\n", meshKey);
		}
	}

	void cModel::LoadMaterials(const aiScene* i_scene, const char* i_matName)
	{

		cMaterial* _newMat = nullptr;
		std::string _path = Assets::ProcessPathMat(i_matName);
		if (!cMaterial::Load(_path, _newMat)) {
			printf("Fail to load material file[%s]\n", _path.c_str());
			return;
		}
		m_materialList.push_back(_newMat);
	}

}