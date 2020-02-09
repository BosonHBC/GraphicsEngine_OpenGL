#pragma once
#include "Externals/Lua/Includes.h"
#include <string>
namespace Assets {
	// ReleaseLUA must be called when a LUA table is initialized
	bool InitializeLUA(const char* i_path, lua_State*& io_luaState);
	bool ReleaseLUA(lua_State*& io_luaState);

	// Loading helper
	bool Lua_LoadInteger(lua_State* i_luaState, const char* i_key, int& o_value);
	bool Lua_LoadFloat(lua_State* i_luaState, const char* i_key, float& o_value);
	bool Lua_LoadVec3(lua_State* i_luaState, const char* i_key, float* o_vec3);
	bool Lua_LoadString(lua_State* i_luaState, const char* i_key, std::string& o_string);
}

