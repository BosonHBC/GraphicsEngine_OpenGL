#include "MatCubemap.h"
#include "Assets/LoadTableFromLuaFile.h"
#include "Graphics/Graphics.h"

namespace Graphics {
	
	bool cMatCubemap::Initialize(const std::string& i_path)
	{
		auto result = true;
		std::vector<std::string> _cubemapTexures;
		_cubemapTexures.resize(6);
		// load material data from LUA files
		if (!(result = LoadFileFromLua(i_path, _cubemapTexures))) {
			printf("Fail to load material[%s] from LUA.\n", i_path.c_str());
			return result;
		}
		std::string _key = i_path;
		_key.append("_cubemapTexture");
		if (result = cTexture::s_manager.Load(_key, m_cubeMapHandle, ETextureType::ETT_CUBEMAP)) 
		{
			cTexture* _texture = cTexture::s_manager.Get(m_cubeMapHandle);
			if (!(result = _texture->LoadCubemap(_cubemapTexures))) {
				printf("Fail to load cube map textures.\n");
				return result;
			}
		}
		else {
			printf("Fail to create cube map textures.\n");
			return result;
		}
		// After loading data from lua, set up uniform variables
		if (!(result = UpdateUniformVariables(Graphics::GetEffectByKey(EET_Cubemap)->GetProgramID())))
		{
			printf("Fail to Update uniform ID\n");
			return result;
		}
		return result;
	}

	bool cMatCubemap::UpdateUniformVariables(GLuint i_programID)
	{
		bool result = true;

		m_cubemapTexID = glGetUniformLocation(i_programID, "cubemapTex");

		return result;
	}

	void cMatCubemap::UseMaterial()
	{
		glUniform1i(m_cubemapTexID, 0);
		cTexture* _handleTex = cTexture::s_manager.Get(m_cubeMapHandle);
		if (_handleTex) {
			_handleTex->UseTexture(GL_TEXTURE0);
		}
		else
			cTexture::UnBindTexture(GL_TEXTURE0, ETT_CUBEMAP);
	}

	void cMatCubemap::CleanUpMaterialBind()
	{
		cTexture::UnBindTexture(GL_TEXTURE0, ETT_CUBEMAP);
	}

	void cMatCubemap::CleanUp()
	{
		cTexture::s_manager.Release(m_cubeMapHandle);
	}

	void cMatCubemap::UpdateCubemap(const Assets::cHandle<cTexture>& i_newTexture)
	{
		cTexture::s_manager.Release(m_cubeMapHandle);
		cTexture::s_manager.Copy(i_newTexture, m_cubeMapHandle);
	}

	bool cMatCubemap::LoadFileFromLua(const std::string& i_path, std::vector<std::string>& o_textures)
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

		// o_textures[0]
		{
			constexpr auto* const _key = "tex_PosX";
			if (!(result = Assets::Lua_LoadString(luaState, _key, o_textures[0]))) {
				printf("LUA error: fail to load key[%s]", _key);
				return result;
			}
		}
		// o_textures[1]
		{
			constexpr auto* const _key = "tex_NegX";
			if (!(result = Assets::Lua_LoadString(luaState, _key, o_textures[1]))) {
				printf("LUA error: fail to load key[%s]", _key);
				return result;
			}
		}		
		// o_textures[2]
		{
			constexpr auto* const _key = "tex_PosY";
			if (!(result = Assets::Lua_LoadString(luaState, _key, o_textures[2]))) {
				printf("LUA error: fail to load key[%s]", _key);
				return result;
			}
		}		
		// o_textures[3]
		{
			constexpr auto* const _key = "tex_NegY";
			if (!(result = Assets::Lua_LoadString(luaState, _key, o_textures[3]))) {
				printf("LUA error: fail to load key[%s]", _key);
				return result;
			}
		}		
		// o_textures[4]
		{
			constexpr auto* const _key = "tex_PosZ";
			if (!(result = Assets::Lua_LoadString(luaState, _key, o_textures[4]))) {
				printf("LUA error: fail to load key[%s]", _key);
				return result;
			}
		}		
		// o_textures[5]
		{
			constexpr auto* const _key = "tex_NegZ";
			if (!(result = Assets::Lua_LoadString(luaState, _key, o_textures[5]))) {
				printf("LUA error: fail to load key[%s]", _key);
				return result;
			}
		}

		//------------------------------
		// Release Lua
		//------------------------------
		result = Assets::ReleaseLUA(luaState);
		return result;
	}

}