#include "MatBlinn.h"
#include "Assets/LoadTableFromLuaFile.h"
#include "Graphics/UniformBuffer/UniformBufferFormats.h"
#include "Assets/PathProcessor.h"
#include "Constants/Constants.h"

namespace Graphics {
	// Definition of static blinnPhongUniformBlock
	cUniformBuffer cMatBlinn::s_BlinnPhongUniformBlock(eUniformBufferType::UBT_BlinnPhongMaterial);

	bool cMatBlinn::Initialize(const std::string& i_path)
	{
		bool result = true;
		std::string _diffusePath, _specularPath, _normalPath;
		// load material data from LUA files
		if (!(result = LoadFileFromLua(i_path, m_matType, _diffusePath, _specularPath, _normalPath, m_diffuseIntensity, m_specularIntensity, m_environmentIntensity, m_shininess))) {
			printf("Fail to load material[%s] from LUA.\n", i_path.c_str());
			return result;
		}

		SetDiffuse(_diffusePath);
		SetSpecular(_specularPath);
		SetNormal(_normalPath);

		if (!(result = s_BlinnPhongUniformBlock.Initialize(nullptr))) {
			printf("Fail to initialize uniformBuffer_BlinnPhong\n");
			return result;
		}
		else {
			s_BlinnPhongUniformBlock.Bind();
		}
		return result;
	}

	bool cMatBlinn::UpdateUniformVariables(GLuint i_programID)
	{
		bool result = true;

		m_diffuseTexID = glGetUniformLocation(i_programID, "diffuseTex");
		m_specularTexID = glGetUniformLocation(i_programID, "specularTex");
		m_cubemapTexID = glGetUniformLocation(i_programID, "cubemapTex");
		m_reflectionTexID = glGetUniformLocation(i_programID, "reflectionTex");
		m_normalTexID = glGetUniformLocation(i_programID, "normalTex");

		return result;
	}

	void cMatBlinn::UseMaterial()
	{
		glUniform1i(m_diffuseTexID, 0);
		cTexture* _diffuseTex = cTexture::s_manager.Get(m_diffuseTextureHandle);
		if (_diffuseTex)
			_diffuseTex->UseTexture(GL_TEXTURE0);
		else
			cTexture::UnBindTexture(GL_TEXTURE0, ETT_FILE);

		glUniform1i(m_specularTexID, 1);
		cTexture* _specularTex = cTexture::s_manager.Get(m_specularTextureHandle);
		if (_specularTex)
			_specularTex->UseTexture(GL_TEXTURE1);
		else
			cTexture::UnBindTexture(GL_TEXTURE1, ETT_FILE);

		glUniform1i(m_normalTexID, 2);
		cTexture* _normalTex = cTexture::s_manager.Get(m_normalTextureHandle);
		if (_normalTex)
			_normalTex->UseTexture(GL_TEXTURE2);
		else
			cTexture::UnBindTexture(GL_TEXTURE2, ETT_FILE);

		glUniform1i(m_cubemapTexID, 3);
		cTexture* _cubemapTex = cTexture::s_manager.Get(m_cubemapTextureHandle);
		if (_cubemapTex)
			_cubemapTex->UseTexture(GL_TEXTURE3);
		else
			cTexture::UnBindTexture(GL_TEXTURE3, ETT_CUBEMAP);

		glUniform1i(m_reflectionTexID, 4);
		cTexture* _reflectionTex = cTexture::s_manager.Get(m_reflectionTextureHandle);
		if (_reflectionTex)
			_reflectionTex->UseTexture(GL_TEXTURE4);
		else
			cTexture::UnBindTexture(GL_TEXTURE4, ETT_FRAMEBUFFER_PLANNER_REFLECTION);

		s_BlinnPhongUniformBlock.Update(&UniformBufferFormats::sBlinnPhongMaterial(m_diffuseIntensity, m_specularIntensity, m_environmentIntensity, m_shininess));
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
		cTexture* _normalTex = cTexture::s_manager.Get(m_normalTextureHandle);
		if (_normalTex) {
			_normalTex->CleanUpTextureBind(GL_TEXTURE2);
		}
		cTexture* _cubemapTex = cTexture::s_manager.Get(m_cubemapTextureHandle);
		if (_cubemapTex) {
			_cubemapTex->CleanUpTextureBind(GL_TEXTURE3);
		}
		cTexture* _reflectionTex = cTexture::s_manager.Get(m_reflectionTextureHandle);
		if (_reflectionTex) {
			_reflectionTex->CleanUpTextureBind(GL_TEXTURE4);
		}
	}

	void cMatBlinn::CleanUp()
	{
		cTexture::s_manager.Release(m_diffuseTextureHandle);
		cTexture::s_manager.Release(m_specularTextureHandle);
		cTexture::s_manager.Release(m_cubemapTextureHandle);
		cTexture::s_manager.Release(m_reflectionTextureHandle);
		cTexture::s_manager.Release(m_normalTextureHandle);

	}
	void cMatBlinn::SetDiffuse(const std::string& i_diffusePath)
	{
		auto result = true;
		std::string _path = Assets::ProcessPathTex(i_diffusePath);
		if (!(result = cTexture::s_manager.Load(_path, m_diffuseTextureHandle, ETextureType::ETT_FILE))) {
			printf("Texture[%s] for diffuse map is invalid, use default color texture instead.\n", _path.c_str());
			//Use default texture, which is the white board
			if (!(result = cTexture::s_manager.Load(Constants::CONST_PATH_DEFAULT_COLOR, m_diffuseTextureHandle, ETextureType::ETT_FILE)))
			{
				printf("Fail to load default texture.\n");
			}
		}
	}

	void cMatBlinn::SetSpecular(const std::string& i_specularPath)
	{
		auto result = true;
		std::string _path = Assets::ProcessPathTex(i_specularPath);
		if (!(result = cTexture::s_manager.Load(_path, m_specularTextureHandle, ETextureType::ETT_FILE))) {
			printf("Texture[%s] for specular map is invalid, use default color texture instead.\n", _path.c_str());
			//Use default texture, which is the white board
			if (!(result = cTexture::s_manager.Load(Constants::CONST_PATH_DEFAULT_COLOR, m_specularTextureHandle, ETextureType::ETT_FILE)))
			{
				printf("Fail to load default color texture.\n");
			}
		}
	}

	void cMatBlinn::SetNormal(const std::string& i_normalPath)
	{
		auto result = true;
		std::string _path = Assets::ProcessPathTex(i_normalPath);
		if (!(result = cTexture::s_manager.Load(_path, m_normalTextureHandle, ETextureType::ETT_FILE))) {
			// Can not load normal texture, use default normal instead
			printf("Texture[%s] for normal map is invalid, use default normal texture instead.\n", _path.c_str());
			// Can not load normal texture, use default normal instead
			if (!(result = cTexture::s_manager.Load(Constants::CONST_PATH_DEFAULT_NORMAL, m_normalTextureHandle, ETextureType::ETT_FILE)))
			{
				printf("Fail to load default normal texture.\n");
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

	void cMatBlinn::UpdateDiffuseTexture(const Assets::cHandle<cTexture>& i_other)
	{
		// release current handle
		cTexture::s_manager.Release(m_diffuseTextureHandle);
		// Copy from incoming texture handle
		cTexture::s_manager.Copy(i_other, m_diffuseTextureHandle);
	}

	void cMatBlinn::UpdateSpecularTexture(const Assets::cHandle<cTexture>& i_other)
	{
		// release current handle
		cTexture::s_manager.Release(m_specularTextureHandle);
		// Copy from incoming texture handle
		cTexture::s_manager.Copy(i_other, m_specularTextureHandle);
	}

	void cMatBlinn::UpdateCubemapTexture(const Assets::cHandle<cTexture>& i_other)
	{
		// release current handle
		cTexture::s_manager.Release(m_cubemapTextureHandle);
		// Copy from incoming texture handle
		cTexture::s_manager.Copy(i_other, m_cubemapTextureHandle);
	}

	void cMatBlinn::UpdateReflectionTexture(const Assets::cHandle<cTexture>& i_other)
	{
		// release current handle
		cTexture::s_manager.Release(m_reflectionTextureHandle);
		// Copy from incoming texture handle
		cTexture::s_manager.Copy(i_other, m_reflectionTextureHandle);
	}

	bool cMatBlinn::LoadFileFromLua(const std::string& i_path, eMaterialType& o_matType, std::string& o_diffusePath, std::string& o_specularPath, std::string& o_normalPath, Color& o_diffuseColor, Color& o_specularColor, Color& o_environmentIntensity, float& o_shineness)
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
			// o_texturePath
			{
				constexpr auto* const _key = "NormalTexture";
				if (!(result = Assets::Lua_LoadString(luaState, _key, o_normalPath))) {
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
			// o_environmentIntensity
			{
				constexpr auto* const _key = "EnvironmentIntensity";
				float _tempHolder[3] = { 0 };
				if (!(result = Assets::Lua_LoadVec3(luaState, _key, _tempHolder))) {
					printf("LUA error: fail to load key[%s]", _key);
					return result;
				}
				for (uint8_t i = 0; i < 3; ++i)
				{
					o_environmentIntensity[i] = _tempHolder[i];
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