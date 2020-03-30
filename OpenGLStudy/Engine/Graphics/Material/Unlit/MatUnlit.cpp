#include "MatUnlit.h"
#include "Assets/LoadTableFromLuaFile.h"
#include "Graphics/Graphics.h"

namespace Graphics {

	bool cMatUnlit::Initialize(const std::string& i_path)
	{
		auto result = true;
		if (!(result = LoadFileFromLua(i_path, m_unlitColor)))
		{
			printf("Fail to load material[%s] from LUA.\n", i_path.c_str());
			return result;
		}
		// After loading data from lua, set up uniform variables
		if (!(result = UpdateUniformVariables(Graphics::GetEffectByKey(EET_Unlit)->GetProgramID())))
		{
			printf("Fail to Update uniform ID\n");
			return result;
		}
		return result;
	}

	bool cMatUnlit::UpdateUniformVariables(GLuint i_programID)
	{
		m_unLitColorID = glGetUniformLocation(i_programID, "arrowColor");
		return true;
	}

	void cMatUnlit::UseMaterial()
	{
		glUniform3f(m_unLitColorID, m_unlitColor.r, m_unlitColor.g, m_unlitColor.b);
		
	}

	void cMatUnlit::CleanUpMaterialBind()
	{

	}

	void cMatUnlit::CleanUp()
	{

	}

	bool cMatUnlit::LoadFileFromLua(const std::string& i_path, Color& o_unlitColor)
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
		// o_unlitColor
		{
			constexpr auto* const _key = "unitColor";
			float _tempHolder[3] = { 0 };
			if (!(result = Assets::Lua_LoadVec3(luaState, _key, _tempHolder))) {
				printf("LUA error: fail to load key[%s]", _key);
				return result;
			}
			for (uint8_t i = 0; i < 3; ++i)
			{
				o_unlitColor[i] = _tempHolder[i];
			}
		}
		//------------------------------
		// Release Lua
		//------------------------------
		result = Assets::ReleaseLUA(luaState);
		return result;
	}

}