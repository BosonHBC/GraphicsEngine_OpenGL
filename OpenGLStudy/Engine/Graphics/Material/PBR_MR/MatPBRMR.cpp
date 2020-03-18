#include "Graphics/Material/PBR_MR/MatPBRMR.h"
#include "Assets/LoadTableFromLuaFile.h"
#include "Assets/PathProcessor.h"
#include "Constants/Constants.h"
#include "Graphics/Graphics.h"
#include "Graphics/EnvironmentCaptureManager.h"
namespace Graphics
{
	Graphics::cUniformBuffer cMatPBRMR::s_PBRMRUniformBlock(eUniformBufferType::UBT_PBRMR);

	bool cMatPBRMR::Initialize(const std::string& i_path)
	{
		bool result = true;
		std::string _albedoPath, _metallicPath, _roughnessPath, _normalPath;
		// load material data from LUA files
		if (!(result = LoadFileFromLua(i_path, _albedoPath, _metallicPath, _roughnessPath, _normalPath, m_diffuseIntensity, m_metallicIntensity, m_roughnessIntensity, m_ior))) {
			printf("Fail to load PBR_MetallicRoughness[%s] from LUA.\n", i_path.c_str());
			return result;
		}

		SetAlbedo(_albedoPath);
		SetMetallic(_metallicPath);
		SetRoughness(_roughnessPath);
		SetNormal(_normalPath);

		if (!(result = s_PBRMRUniformBlock.Initialize(nullptr))) {
			printf("Fail to initialize uniformBuffer_PBRMMR\n");
			return result;
		}
		else {
			s_PBRMRUniformBlock.Bind();
		}
		return result;
	}

	bool cMatPBRMR::UpdateUniformVariables(GLuint i_programID)
	{
		bool result = true;

		m_albedoID = glGetUniformLocation(i_programID, "AlbedoMap");
		m_metallicID = glGetUniformLocation(i_programID, "MetallicMap");
		m_roughnessID = glGetUniformLocation(i_programID, "RoughnessMap");
		m_normalD = glGetUniformLocation(i_programID, "NormalMap");
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

		glUniform1i(m_normalD, 3);
		cTexture* _normalTex = cTexture::s_manager.Get(m_normalMapHandle);
		if (_normalTex)
			_normalTex->UseTexture(GL_TEXTURE3);
		else
			cTexture::UnBindTexture(GL_TEXTURE3, ETT_FILE);

		// Irradiance cube map
		{
			const cEnvProbe& _irradProbe = Graphics::EnvironmentCaptureManager::GetCaptureProbesAt(0).IrradianceProbe;
			cTexture* _irrdianceMap = nullptr;
			if (_irradProbe.IsValid() && (_irrdianceMap = cTexture::s_manager.Get(_irradProbe.GetCubemapTextureHandle())))
			{
				_irrdianceMap->UseTexture(GL_TEXTURE4);
			}
			else
				cTexture::UnBindTexture(GL_TEXTURE4, ETT_FRAMEBUFFER_HDR_CUBEMAP);
		}

		// pre-filter cube map
		{
			const cEnvProbe& _preFilteProbe = Graphics::EnvironmentCaptureManager::GetCaptureProbesAt(0).PrefilterProbe;
			cTexture* _preFilterCubemap = nullptr;
			if (_preFilteProbe.IsValid() && (_preFilterCubemap = cTexture::s_manager.Get(_preFilteProbe.GetCubemapTextureHandle())))
			{
				_preFilterCubemap->UseTexture(GL_TEXTURE5);
			}
			else
				cTexture::UnBindTexture(GL_TEXTURE5, ETT_FRAMEBUFFER_HDR_MIPMAP_CUBEMAP);
		}

		// BRDF LUT texture
		{
			cFrameBuffer* _lutTextureFrameBuffer = Graphics::GetBRDFLutFrameBuffer();
			cTexture* _lutTexture = nullptr;
			if (_lutTextureFrameBuffer && _lutTextureFrameBuffer->IsValid() && (_lutTexture = cTexture::s_manager.Get(_lutTextureFrameBuffer->GetTextureHandle())))
			{
				_lutTexture->UseTexture(GL_TEXTURE17);
			}
			else
				cTexture::UnBindTexture(GL_TEXTURE17, ETT_FRAMEBUFFER_HDR_RG);
		}

		s_PBRMRUniformBlock.Update(&UniformBufferFormats::sPBRMRMaterial(m_diffuseIntensity, m_roughnessIntensity, m_ior, m_metallicIntensity));
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

		cTexture::UnBindTexture(GL_TEXTURE4, ETT_FRAMEBUFFER_HDR_CUBEMAP);
		cTexture::UnBindTexture(GL_TEXTURE5, ETT_FRAMEBUFFER_HDR_MIPMAP_CUBEMAP);
		cTexture::UnBindTexture(GL_TEXTURE17, ETT_FRAMEBUFFER_HDR_RG);

	}

	void cMatPBRMR::CleanUp()
	{
		cTexture::s_manager.Release(m_albedoMapHandle);
		cTexture::s_manager.Release(m_metallicMapHandle);
		cTexture::s_manager.Release(m_roughnessMapHandle);
		cTexture::s_manager.Release(m_normalMapHandle);
	}

	bool cMatPBRMR::LoadFileFromLua(const std::string& i_path, std::string& o_albedoPath, std::string& o_metallicPath, std::string& o_roughnessPath, std::string& o_normalPath, Color& o_diffuseIntensity, float& o_metallicIntensity, float& o_roughnessIntensity, glm::vec3& o_ior)
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



}