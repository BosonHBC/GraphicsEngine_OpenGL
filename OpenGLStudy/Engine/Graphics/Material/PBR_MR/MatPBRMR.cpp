#include "Graphics/Material/PBR_MR/MatPBRMR.h"
#include "Assets/LoadTableFromLuaFile.h"
#include "Assets/PathProcessor.h"
#include "Constants/Constants.h"
#include "Graphics/Graphics.h"
#include "Graphics/EnvironmentCaptureManager.h"

namespace Graphics
{
	Graphics::cUniformBuffer cMatPBRMR::s_PBRMRUniformBlock(eUniformBufferType::UBT_PBRMR);

	Graphics::cMatPBRMR& cMatPBRMR::operator=(const cMatPBRMR& i_rhs)
	{
		cMaterial::operator=(i_rhs);
		cTexture::s_manager.Copy(i_rhs.m_albedoMapHandle, m_albedoMapHandle);
		cTexture::s_manager.Copy(i_rhs.m_metallicMapHandle, m_metallicMapHandle);
		cTexture::s_manager.Copy(i_rhs.m_roughnessMapHandle, m_roughnessMapHandle);
		cTexture::s_manager.Copy(i_rhs.m_normalMapHandle, m_normalMapHandle);
		cTexture::s_manager.Copy(i_rhs.m_aoMapHandle, m_aoMapHandle);

		DiffuseIntensity = i_rhs.DiffuseIntensity;
		MetallicIntensity = i_rhs.MetallicIntensity;
		RoughnessIntensity = i_rhs.RoughnessIntensity;
		IoR = i_rhs.IoR;

		m_albedoID = i_rhs.m_albedoID;
		m_metallicID = i_rhs.m_metallicID;
		m_roughnessID = i_rhs.m_roughnessID;
		m_normalID = i_rhs.m_normalID;
		m_aoID = i_rhs.m_aoID;

		return *this;
	}

	cMatPBRMR::cMatPBRMR(const cMatPBRMR& i_other): cMaterial(i_other), DiffuseIntensity (i_other.DiffuseIntensity),
	MetallicIntensity (i_other.MetallicIntensity), RoughnessIntensity(i_other.RoughnessIntensity), IoR(i_other.IoR),
		m_albedoID(i_other.m_albedoID), m_metallicID(i_other.m_metallicID), m_roughnessID (i_other.m_roughnessID), m_normalID(i_other.m_normalID), m_aoID(i_other.m_aoID)
	{
		cTexture::s_manager.Copy(i_other.m_albedoMapHandle, m_albedoMapHandle);
		cTexture::s_manager.Copy(i_other.m_metallicMapHandle, m_metallicMapHandle);
		cTexture::s_manager.Copy(i_other.m_roughnessMapHandle, m_roughnessMapHandle);
		cTexture::s_manager.Copy(i_other.m_normalMapHandle, m_normalMapHandle);
		cTexture::s_manager.Copy(i_other.m_aoMapHandle, m_aoMapHandle);
	}

	bool cMatPBRMR::Initialize(const std::string& i_path)
	{
		bool result = true;
		std::string _albedoPath, _metallicPath, _roughnessPath, _normalPath, _aoPath;
		// load material data from LUA files
		if (!(result = LoadFileFromLua(i_path, _albedoPath, _metallicPath, _roughnessPath, _normalPath, _aoPath, DiffuseIntensity, MetallicIntensity, RoughnessIntensity, IoR))) {
			printf("Fail to load PBR_MetallicRoughness[%s] from LUA.\n", i_path.c_str());
			return result;
		}

		SetAlbedo(_albedoPath);
		SetMetallic(_metallicPath);
		SetRoughness(_roughnessPath);
		SetNormal(_normalPath);
		SetAO(_aoPath);

		if (!(result = s_PBRMRUniformBlock.Initialize(nullptr))) {
			printf("Fail to initialize uniformBuffer_PBRMMR\n");
			return result;
		}
		else {
			s_PBRMRUniformBlock.Bind();
		}
		// After loading data from lua, set up uniform variables
		if (!(result = UpdateUniformVariables(Graphics::GetEffectByKey(EET_PBR_MR)->GetProgramID())))
		{
			printf("Fail to Update uniform ID\n");
			return result;
		}
		return result;
	}

	bool cMatPBRMR::UpdateUniformVariables(GLuint i_programID)
	{
		bool result = true;

		m_albedoID = glGetUniformLocation(i_programID, "AlbedoMap");
		m_metallicID = glGetUniformLocation(i_programID, "MetallicMap");
		m_roughnessID = glGetUniformLocation(i_programID, "RoughnessMap");
		m_normalID = glGetUniformLocation(i_programID, "NormalMap");
		m_aoID = glGetUniformLocation(i_programID, "AOMap");
		assert(GL_NO_ERROR == glGetError());
		return result;
	}

	void cMatPBRMR::UseMaterial()
	{
		glUniform1i(m_albedoID, 0);
		cTexture* _albedoMap = cTexture::s_manager.Get(m_albedoMapHandle);
		if (_albedoMap)
			_albedoMap->UseTexture(GL_TEXTURE0);
		else
			cTexture::UnBindTexture(GL_TEXTURE0, ETT_FILE);

		glUniform1i(m_metallicID, 1);
		cTexture* _metallicMap = cTexture::s_manager.Get(m_metallicMapHandle);
		if (_metallicMap)
			_metallicMap->UseTexture(GL_TEXTURE1);
		else
			cTexture::UnBindTexture(GL_TEXTURE1, ETT_FILE_GRAY);

		glUniform1i(m_roughnessID, 2);
		cTexture* _roughnessMap = cTexture::s_manager.Get(m_roughnessMapHandle);
		if (_roughnessMap)
			_roughnessMap->UseTexture(GL_TEXTURE2);
		else
			cTexture::UnBindTexture(GL_TEXTURE2, ETT_FILE_GRAY);

		glUniform1i(m_normalID, 3);
		cTexture* _normalTex = cTexture::s_manager.Get(m_normalMapHandle);
		if (_normalTex)
			_normalTex->UseTexture(GL_TEXTURE3);
		else
			cTexture::UnBindTexture(GL_TEXTURE3, ETT_FILE);

		glUniform1i(m_aoID, 5);
		cTexture* _aoTexture = cTexture::s_manager.Get(m_aoMapHandle);
		if (_aoTexture)
			_aoTexture->UseTexture(GL_TEXTURE5);
		else
			cTexture::UnBindTexture(GL_TEXTURE5, ETT_FILE_GRAY);

		s_PBRMRUniformBlock.Update(&UniformBufferFormats::sPBRMRMaterial(DiffuseIntensity, RoughnessIntensity, IoR, MetallicIntensity));

		assert(GL_NO_ERROR == glGetError());

	}

	void cMatPBRMR::CleanUpMaterialBind()
	{

		cTexture* _albedoMap = cTexture::s_manager.Get(m_albedoMapHandle);
		if (_albedoMap)
			_albedoMap->CleanUpTextureBind(GL_TEXTURE0);

		cTexture* _metallicMap = cTexture::s_manager.Get(m_metallicMapHandle);
		if (_metallicMap)
			_metallicMap->CleanUpTextureBind(GL_TEXTURE1);

		cTexture* _roughnessMap = cTexture::s_manager.Get(m_roughnessMapHandle);
		if (_roughnessMap)
			_roughnessMap->CleanUpTextureBind(GL_TEXTURE2);

		cTexture* _normalTex = cTexture::s_manager.Get(m_normalMapHandle);
		if (_normalTex)
			_normalTex->CleanUpTextureBind(GL_TEXTURE3);

		cTexture* _aoTexture = cTexture::s_manager.Get(m_aoMapHandle);
		if (_aoTexture)
			_aoTexture->CleanUpTextureBind(GL_TEXTURE5);

		assert(GL_NO_ERROR == glGetError());

	}

	void cMatPBRMR::CleanUp()
	{
		cTexture::s_manager.Release(m_albedoMapHandle);
		cTexture::s_manager.Release(m_metallicMapHandle);
		cTexture::s_manager.Release(m_roughnessMapHandle);
		cTexture::s_manager.Release(m_normalMapHandle);
		cTexture::s_manager.Release(m_aoMapHandle);
	}

	bool cMatPBRMR::LoadFileFromLua(const std::string& i_path, std::string& o_albedoPath, std::string& o_metallicPath, std::string& o_roughnessPath, std::string& o_normalPath, std::string& o_aoPath,Color& o_diffuseIntensity, float& o_metallicIntensity, float& o_roughnessIntensity, glm::vec3& o_ior)
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
		// o_albedoPath
		{
			constexpr auto* const _key = "AlbedoTexture";
			if (!(result = Assets::Lua_LoadString(luaState, _key, o_albedoPath))) {
				printf("LUA error: fail to load key[%s]", _key);
				return result;
			}
		}
		// o_metallicPath
		{
			constexpr auto* const _key = "MetallicTexture";
			if (!(result = Assets::Lua_LoadString(luaState, _key, o_metallicPath))) {
				printf("LUA error: fail to load key[%s]", _key);
				return result;
			}
		}
		// o_roughnessPath
		{
			constexpr auto* const _key = "RoughnessTexture";
			if (!(result = Assets::Lua_LoadString(luaState, _key, o_roughnessPath))) {
				printf("LUA error: fail to load key[%s]", _key);
				return result;
			}
		}
		// o_normalPath
		{
			constexpr auto* const _key = "NormalTexture";
			if (!(result = Assets::Lua_LoadString(luaState, _key, o_normalPath))) {
				printf("LUA error: fail to load key[%s]", _key);
				return result;
			}
		}
		// o_aoPath
		{
			constexpr auto* const _key = "AOTexture";
			if (!(result = Assets::Lua_LoadString(luaState, _key, o_aoPath))) {
				printf("LUA error: fail to load key[%s]", _key);
				return result;
			}
		}
		// Diffuse Intensity
		{
			constexpr auto* const _key = "DiffuseIntensity";
			float _tempHolder[3] = { 0 };
			if (!(result = Assets::Lua_LoadVec3(luaState, _key, _tempHolder))) {
				printf("LUA error: fail to load key[%s]", _key);
				return result;
			}
			for (uint8_t i = 0; i < 3; ++i)
			{
				o_diffuseIntensity[i] = _tempHolder[i];
			}
		}
		// Metallic Intensity
		{
			constexpr auto* const _key = "MetallicIntensity";
			if (!(result = Assets::Lua_LoadFloat(luaState, _key, o_metallicIntensity))) {
				printf("LUA error: fail to load key[%s]", _key);
				return result;
			}
		}
		// Roughness Intensity
		{
			constexpr auto* const _key = "RoughnessIntensity";
			if (!(result = Assets::Lua_LoadFloat(luaState, _key, o_roughnessIntensity))) {
				printf("LUA error: fail to load key[%s]", _key);
				return result;
			}
		}
		// IOR
		{
			constexpr auto* const _key = "IOR";
			float _tempHolder[3] = { 0 };
			if (!(result = Assets::Lua_LoadVec3(luaState, _key, _tempHolder))) {
				printf("LUA error: fail to load key[%s]", _key);
				return result;
			}
			for (uint8_t i = 0; i < 3; ++i)
			{
				o_ior[i] = _tempHolder[i];
			}
		}
		//------------------------------
		// Release Lua
		//------------------------------
		result = Assets::ReleaseLUA(luaState);
		return result;
	}

	bool cMatPBRMR::SetAlbedo(const std::string& i_path)
	{
		auto result = true;

		std::string _path = Assets::ProcessPathTex(i_path);
		if (!(result = cTexture::s_manager.Load(_path, m_albedoMapHandle, ETextureType::ETT_FILE))) {
			printf("Texture[%s] for diffuse map is invalid, use default color texture instead.\n", _path.c_str());
			//Use default texture, which is the white board
			if (!(result = cTexture::s_manager.Load(Constants::CONST_PATH_DEFAULT_COLOR, m_albedoMapHandle, ETextureType::ETT_FILE)))
			{
				printf("Fail to load default texture.\n");
			}
		}

		return result;
	}

	bool cMatPBRMR::SetMetallic(const std::string& i_path)
	{
		auto result = true;
		std::string _path = Assets::ProcessPathTex(i_path);
		if (!(result = cTexture::s_manager.Load(_path, m_metallicMapHandle, ETextureType::ETT_FILE_GRAY))) {
			printf("Texture[%s] for metallic map is invalid, use default color texture instead.\n", _path.c_str());
			//Use default texture, which is the white board
			if (!(result = cTexture::s_manager.Load(Constants::CONST_PATH_DEFAULT_COLOR, m_metallicMapHandle, ETextureType::ETT_FILE_GRAY)))
			{
				printf("Fail to load default texture.\n");
			}
		}
		return result;
	}

	bool cMatPBRMR::SetRoughness(const std::string& i_path)
	{
		auto result = true;
		std::string _path = Assets::ProcessPathTex(i_path);
		if (!(result = cTexture::s_manager.Load(_path, m_roughnessMapHandle, ETextureType::ETT_FILE_GRAY))) {
			printf("Texture[%s] for roughness map is invalid, use default color texture instead.\n", _path.c_str());
			//Use default texture, which is the white board
			if (!(result = cTexture::s_manager.Load(Constants::CONST_PATH_DEFAULT_COLOR, m_roughnessMapHandle, ETextureType::ETT_FILE_GRAY)))
			{
				printf("Fail to load default texture.\n");
			}
		}
		return result;
	}

	bool cMatPBRMR::SetNormal(const std::string& i_path)
	{
		auto result = true;
		std::string _path = Assets::ProcessPathTex(i_path);
		if (!(result = cTexture::s_manager.Load(_path, m_normalMapHandle, ETextureType::ETT_FILE))) {
			// Can not load normal texture, use default normal instead
			printf("Texture[%s] for normal map is invalid, use default normal texture instead.\n", _path.c_str());
			// Can not load normal texture, use default normal instead
			if (!(result = cTexture::s_manager.Load(Constants::CONST_PATH_DEFAULT_NORMAL, m_normalMapHandle, ETextureType::ETT_FILE)))
			{
				printf("Fail to load default normal texture.\n");
			}
		}
		return result;
	}



	bool cMatPBRMR::SetAO(const std::string& i_path)
	{
		auto result = true;
		std::string _path = Assets::ProcessPathTex(i_path);
		if (!(result = cTexture::s_manager.Load(_path, m_aoMapHandle, ETextureType::ETT_FILE_GRAY))) {
			printf("Texture[%s] for ao map is invalid, use default color texture instead.\n", _path.c_str());
			//Use default texture, which is the white board
			if (!(result = cTexture::s_manager.Load(Constants::CONST_PATH_DEFAULT_COLOR, m_aoMapHandle, ETextureType::ETT_FILE_GRAY)))
			{
				printf("Fail to load default texture.\n");
			}
		}
		return result;
	}

}