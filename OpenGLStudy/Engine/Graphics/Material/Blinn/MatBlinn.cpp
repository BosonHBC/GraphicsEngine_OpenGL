#include "MatBlinn.h"
#include "Engine/Constants/Constants.h"
#include "assimp/scene.h"
#include "Assets/LoadTableFromLuaFile.h"

namespace Graphics {


	bool cMatBlinn::Initialize(const std::string& i_path)
	{
		bool result = true;
		std::string _diffusePath, _specularPath;
		// load material data from LUA files
		if (!(result = LoadFileFromLua(i_path, m_matType,_diffusePath, _specularPath, m_diffuseIntensity, m_specularIntensity, m_shininess))) {
			printf("Fail to load material[%s] from LUA.\n", i_path.c_str());
			return result;
		}

		SetDiffuse(_diffusePath);
		SetSpecular(_specularPath);
		return result;
	}

	bool cMatBlinn::UpdateUniformVariables(GLuint i_programID)
	{
		bool result = true;

		m_diffuseTexID = glGetUniformLocation(i_programID, "diffuseTex");
		m_specularTexID = glGetUniformLocation(i_programID, "specularTex");

		m_shininessID = glGetUniformLocation(i_programID, "material.shininess");
		m_diffuseIntensityID = glGetUniformLocation(i_programID, "material.kd");
		m_specularIntensityID = glGetUniformLocation(i_programID, "material.ks");
		return result;
	}

	void cMatBlinn::UseMaterial()
	{
		glUniform1i(m_diffuseTexID, 0);
		glUniform1i(m_specularTexID, 1);

		cTexture* _diffuseTex = cTexture::s_manager.Get(m_diffuseTextureHandle);
		if (_diffuseTex) {
			_diffuseTex->UseTexture(GL_TEXTURE0);
		}
		cTexture* _specularTex = cTexture::s_manager.Get(m_specularTextureHandle);
		if (_specularTex) {
			_specularTex->UseTexture(GL_TEXTURE1);
		}

		glUniform1f(m_shininessID, m_shininess);
		glUniform3f(m_diffuseIntensityID, m_diffuseIntensity.r, m_diffuseIntensity.g, m_diffuseIntensity.b);
		glUniform3f(m_specularIntensityID, m_specularIntensity.r, m_specularIntensity.g, m_specularIntensity.b);

	}

	void cMatBlinn::CleanUpMaterialBind()
	{
		cTexture* _diffuseTex = cTexture::s_manager.Get(m_diffuseTextureHandle);
		if (_diffuseTex) {
			_diffuseTex->CleanUpTextureBind(GL_TEXTURE0);
		}
		cTexture* _specularTex = cTexture::s_manager.Get(m_specularTextureHandle);
		if (_specularTex) {
			_specularTex->CleanUpTextureBind(GL_TEXTURE1);
		}
	}

	void cMatBlinn::CleanUp()
	{
		cTexture::s_manager.Release(m_diffuseTextureHandle);
		cTexture::s_manager.Release(m_specularTextureHandle);

	}
	void cMatBlinn::SetDiffuse(const std::string& i_diffusePath)
	{
		auto result = true;
		std::string _path = i_diffusePath;
		if (!(result = cTexture::s_manager.Load(_path.insert(0, Constants::CONST_PATH_TEXTURE_ROOT), m_diffuseTextureHandle, false))) {
			if (result = cTexture::s_manager.Load(Constants::CONST_PATH_DEFAULT_TEXTURE, m_diffuseTextureHandle, false))
			{
				//TODO: Use default texture, which is the white board

			}
			else {
				//TODO: print Fail to load default texture

			}
		}
	}

	void cMatBlinn::SetSpecular(const std::string& i_specularPath)
	{
		auto result = true;
		std::string _path = i_specularPath;
		if (!(result = cTexture::s_manager.Load(_path.insert(0, Constants::CONST_PATH_TEXTURE_ROOT), m_specularTextureHandle, false))) {
			if (result = cTexture::s_manager.Load(Constants::CONST_PATH_DEFAULT_TEXTURE, m_specularTextureHandle, false))
			{
				//TODO: Use default texture, which is the white board
				printf("Texture[%s] is invalid, use default texture instead.\n",_path.c_str());
			}
			else {
				//TODO: print Fail to load default texture
				printf("Fail to load default texture.\n");
			}
		}
	}

	void cMatBlinn::SetShininess(GLfloat i_shine)
	{
		m_shininess = i_shine;
	}

	void cMatBlinn::SetDiffuseIntensity(Color i_diffuseIntensity)
	{
		m_diffuseIntensity = i_diffuseIntensity;
	}

	void cMatBlinn::SetSpecularIntensity(Color i_specularIntensity)
	{
		m_specularIntensity = i_specularIntensity;
	}

	bool cMatBlinn::LoadFileFromLua(const std::string& i_path, eMaterialType& o_matType, std::string& o_diffusePath, std::string& o_specularPath, Color& o_diffuseColor, Color& o_specularColor, float& o_shineness)
	{
		bool result;
		lua_State* luaState = nullptr;
		//------------------------------
		// Initialize Lua
		//------------------------------
		if (!(result = Assets::InitializeLUA(i_path.c_str(), luaState))) {
			Assets::ReleaseLUA(luaState);
			return result;
		}
		//------------------------------
		// Load data
		//------------------------------
		{
			// o_matType
			{
				constexpr auto* const _key = "MaterialType";
				int _tempType = 0;
				if (!(result = Assets::Lua_LoadInteger(luaState, _key, _tempType))) {
					printf("LUA error: fail to load key[%s]", _key);
					return result;
				}
			}
			// o_diffusePath
			{
				constexpr auto* const _key = "DiffuseTexture";
				if (!(result = Assets::Lua_LoadString(luaState, _key, o_diffusePath))) {
					printf("LUA error: fail to load key[%s]", _key);
					return result;
				}
			}
			// o_specularPath
			{
				constexpr auto* const _key = "SpecularTexture";
				if (!(result = Assets::Lua_LoadString(luaState, _key, o_specularPath))) {
					printf("LUA error: fail to load key[%s]", _key);
					return result;
				}
			}
			// o_diffuseColor
			{
				constexpr auto* const _key = "DiffuseIntensity";
				float _tempHolder[3] = { 0 };
				if (!(result = Assets::Lua_LoadVec3(luaState, _key, _tempHolder))) {
					printf("LUA error: fail to load key[%s]", _key);
					return result;
				}
				for (uint8_t i = 0; i < 3; ++i)
				{
					o_diffuseColor[i] = _tempHolder[i];
				}
			}
			// o_specularColor
			{
				constexpr auto* const _key = "SpecularIntensity";
				float _tempHolder[3] = { 0 };
				if (!(result = Assets::Lua_LoadVec3(luaState, _key, _tempHolder))) {
					printf("LUA error: fail to load key[%s]", _key);
					return result;
				}
				for (uint8_t i = 0; i < 3; ++i)
				{
					o_specularColor[i] = _tempHolder[i];
				}
			}
			// o_shineness
			{
				constexpr auto* const _key = "Shininess";
				if (!(result = Assets::Lua_LoadFloat(luaState, _key, m_shininess))) {
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

}