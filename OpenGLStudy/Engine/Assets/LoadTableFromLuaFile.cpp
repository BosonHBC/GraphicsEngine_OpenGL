#include "LoadTableFromLuaFile.h"
namespace Assets {

	bool InitializeLUA(const char* i_path, lua_State*& io_luaState)
	{
		auto result = true;
		// -------------------------------------
		// Create a new Lua state
		// -------------------------------------
		if (io_luaState) {
			result = false;
			// TODO: print error
			printf("Input LUA state has been initialized. Can not initialized again \n");
			return result;
		}
		io_luaState = reinterpret_cast<lua_State*>(io_luaState);
		{
			io_luaState = luaL_newstate();
			if (!io_luaState)
			{
				result = false;
				// TODO: print error
				printf("Can not create LUA table because there is not enough memory!\n");
				return result;
			}
		}

		// Load the asset file as a "chunk",
		// meaning there will be a callable function at the top of the stack
		const auto stackTopBeforeLoad = lua_gettop(io_luaState);
		// -------------------------------------
		// Initialize Lua state
		// -------------------------------------
		{
			const auto luaResult = luaL_loadfile(io_luaState, i_path);
			if (luaResult != LUA_OK)
			{
				result = false;
				printf("LUA error: %s\n", lua_tostring(io_luaState, -1));
				// Pop the error message
				lua_pop(io_luaState, 1);
				return result;
			}
		}
		// -------------------------------------
		// Execute the "chunk", which should load the asset into a table at the top of the stack
		// -------------------------------------
		{
			constexpr int argumentCount = 0;
			constexpr int returnValueCount = LUA_MULTRET;	// Return _everything_ that the file returns
			constexpr int noMessageHandler = 0;
			const auto luaResult = lua_pcall(io_luaState, argumentCount, returnValueCount, noMessageHandler);
			if (luaResult == LUA_OK)
			{
				// A well-behaved asset file will only return a single value
				const auto returnedValueCount = lua_gettop(io_luaState) - stackTopBeforeLoad;
				if (returnedValueCount == 1)
				{
					// A correct asset file _must_ return a table
					if (!lua_istable(io_luaState, -1))
					{
						result = false;
						printf("LUA error: %s", "Asset files must return a table\n");
						// Pop the returned non-table value
						lua_pop(io_luaState, 1);
						return result;
					}
				}
				else
				{
					result = false;
					printf("LUA error: %s", "Asset files must return a single table\n");
					// Pop every value that was returned
					lua_pop(io_luaState, returnedValueCount);
					return result;
				}
			}
			else
			{
				result = false;
				printf("LUA error: %s", lua_tostring(io_luaState, -1));
				// Pop the error message
				lua_pop(io_luaState, 1);
				return result;
			}
		}
		printf("LUA initialized successfully.\n");
		return result;
	}

	bool ReleaseLUA(lua_State*& io_luaState)
	{
		auto result = true;
		if (io_luaState) {
			// If this code is reached the asset file was loaded successfully,
			// and its table is now at index -1
			lua_pop(io_luaState, 1);

			if (io_luaState)
			{
				// till here, LUA table should be clean
				if (lua_gettop(io_luaState) != 0)
				{
					// TODO: print error
					printf("LUA is not on the top now\n");
					result = false;
				}
				lua_close(io_luaState);
				io_luaState = nullptr;
			}
		}
		else {
			result = false;
			printf("LUA error: Can not close an empty lua table");
			return false;
		}
		printf("LUA released successfully.\n");
		return result;
	}

	bool Lua_LoadInteger(lua_State* i_luaState, const char* i_key, int& o_value)
	{
		auto result = true;
		lua_pushstring(i_luaState, i_key);
		lua_gettable(i_luaState, -2);
		if (!(result = lua_isinteger(i_luaState, -1)))
		{
			printf("LUA error: invalid data type in key[%s]\n", i_key);
			lua_pop(i_luaState, 1);
			return result;
		}
		o_value = static_cast<int>(lua_tointeger(i_luaState, -1));
		lua_pop(i_luaState, 1);
		return result;
	}

	bool Lua_LoadFloat(lua_State* i_luaState, const char* i_key, float& o_value)
	{
		auto result = true;
		lua_pushstring(i_luaState, i_key);
		lua_gettable(i_luaState, -2);
		if (!(result = lua_isnumber(i_luaState, -1)))
		{
			printf("LUA error: invalid data type in key[%s]\n", i_key);
			lua_pop(i_luaState, 1);
			return result;
		}
		o_value = static_cast<float>(lua_tonumber(i_luaState, -1));
		lua_pop(i_luaState, 1);
		return result;
	}

	bool Lua_LoadVec3(lua_State* i_luaState, const char* i_key, float* o_vec3)
	{
		auto result = true;
		lua_pushstring(i_luaState, i_key);
		lua_gettable(i_luaState, -2);
		if (!(result = lua_istable(i_luaState, -1)))
		{
			printf("LUA error: invalid data type in key[%s]\n", i_key);
			lua_pop(i_luaState, 1);
			return result;
		}
		// load color index
		for (size_t i = 1; i < 4; ++i)
		{
			lua_pushinteger(i_luaState, i);
			lua_gettable(i_luaState, -2);

			o_vec3[i - 1] = static_cast<float>(lua_tonumber(i_luaState, -1));
			lua_pop(i_luaState, 1);
		}
		lua_pop(i_luaState, 1);
		return result;
	}

	bool Lua_LoadString(lua_State* i_luaState, const char* i_key, std::string& o_string)
	{
		auto result = true;
		lua_pushstring(i_luaState, i_key);
		lua_gettable(i_luaState, -2);
		if (!(result = lua_isstring(i_luaState, -1)))
		{
			printf("LUA error: invalid data type in key[%s]\n", i_key);
			lua_pop(i_luaState, 1);
			return result;
		}
		o_string = static_cast<std::string>(lua_tostring(i_luaState, -1));
		lua_pop(i_luaState, 1);
		return result;
	}

}