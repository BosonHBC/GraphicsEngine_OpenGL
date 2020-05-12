#include "Engine/Graphics/Model/Model.h"
#include "Engine/Graphics/Material/Material.h"

#include "Engine/Constants/Constants.h"

#include "Externals/ASSIMP_N/include/assimp/Importer.hpp"
#include "Externals/ASSIMP_N/include/assimp/scene.h"
#include "Externals/ASSIMP_N/include/assimp/postprocess.h"


#include "Assets/LoadTableFromLuaFile.h"
#include "Assets/PathProcessor.h"
#include "Math/Transform/Transform.h"
#include "Cores/Actor/Actor.h"
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
			| aiProcess_GenBoundingBoxes
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

	void cModel::Render(GLenum i_drawMode/* = GL_TRIANGLES*/) const
	{
		//for (size_t i = 0; i < m_meshList.size(); ++i)
		{
			cMaterial* _material = cMaterial::s_manager.Get(m_materialList[0]);
			// if _maxIndex is in range and the texture is not a nullptr
			if (_material) {
				_material->UseMaterial();
			}

			auto _mesh = cMesh::s_manager.Get(m_meshList[0]);
			if (_mesh)
				_mesh->Render(i_drawMode);

			if (_material) {
				_material->CleanUpMaterialBind();
			}
		}
	}

	void cModel::RenderWithoutMaterial(GLenum i_drawMode/* = GL_TRIANGLES*/) const
	{
		//for (size_t i = 0; i < m_meshList.size(); ++i)
		//{
		auto _mesh = cMesh::s_manager.Get(m_meshList[0]);
		if (_mesh)
			_mesh->Render(i_drawMode);
		//}
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
			cMesh::s_manager.Release(m_meshList[i]);
		}
		m_meshList.clear();
		m_meshList.~vector();

		DecreamentSelectableCount();
	}

	bool cModel::IntersectWithSphere(const cSphere& i_transformedSphere)
	{
		for (auto it : m_meshList)
		{
			cMesh* _mesh = cMesh::s_manager.Get(it);
			if (_mesh && _mesh->IntersectWithSphere(i_transformedSphere))
				return true;
		}
		return false;
	}

	Graphics::cMaterial::HANDLE cModel::GetMaterialAt(GLuint i_idx /*= 0*/)
	{
		if (i_idx < m_materialList.size()) {
			return m_materialList[i_idx];
		}
		assert(false);
		return cMaterial::HANDLE();
	}



	bool cModel::GetBoundTransform(cTransform *& o_transform)
	{
		if (!m_owner) return false;
		return (o_transform = &(m_owner->Transform));
	}

	cModel::cModel(const std::string& i_path)
	{
		LoadModel(i_path.c_str());
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
		glm::vec3 _aabbMin, _aabbMax;
		_aabbMin.x = i_mesh->mAABB.mMin.x; _aabbMin.y = i_mesh->mAABB.mMin.y; _aabbMin.z = i_mesh->mAABB.mMin.z;
		_aabbMax.x = i_mesh->mAABB.mMax.x; _aabbMax.y = i_mesh->mAABB.mMax.y; _aabbMax.z = i_mesh->mAABB.mMax.z;

		cMesh::HANDLE _newMeshHandle;
		std::string meshKey = std::string(i_path).append("_" + std::to_string(m_meshList.size()));
		if (cMesh::s_manager.Load(meshKey, _newMeshHandle, EMT_Mesh, _vertices, _indices, _aabbMin, _aabbMax))
		{
			// Stored new mesh to the list and store its index to material
			m_meshList.push_back(_newMeshHandle);
		}
		else
		{
			printf("Loading Model fail: Fail to create mesh%s\n", meshKey.c_str());
		}
	}

	void cModel::LoadMaterials(const aiScene* i_scene, const char* i_matName)
	{

		cMaterial::HANDLE _newMat;
		std::string _path = Assets::ProcessPathMat(i_matName);
		if (!cMaterial::s_manager.Load(_path, _newMat)) {
			printf("Fail to load material file[%s]\n", _path.c_str());
			return;
		}
		m_materialList.push_back(_newMat);
	}

	void cModel::UpdateMaterial(const cMaterial::HANDLE& i_mat)
	{
		cMaterial::s_manager.Release(m_materialList[0]);
		m_materialList[0] = i_mat;
	}

}