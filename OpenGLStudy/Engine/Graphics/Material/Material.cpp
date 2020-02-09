#include "Blinn/MatBlinn.h"
#include "assimp/scene.h"
#include "Externals/Lua/Includes.h"

namespace Graphics {

	Assets::cAssetManager < cMaterial > cMaterial::s_manager;

	bool cMaterial::Load(const std::string& i_path, cMaterial*& o_material, aiMaterial* const i_aiMat)
	{
		auto result = true;

		cMaterial* _mat = nullptr;
		// TODO: read the material type from LUA file, right now, set it to blinn-phong
		eMaterialType _matType = MT_BLINN_PHONG;

		switch (_matType)
		{
		case eMaterialType::MT_INVALID:

			// TODO: invalid material type
			result = false;
			return result;
		case eMaterialType::MT_BLINN_PHONG:
			_mat = new (std::nothrow) cMatBlinn();
			break;
		default:
			break;
		}

		if (!(result = _mat)) {
			// Run out of memory
			// TODO: LogError: Out of memory

			return result;
		}

		if (!(result = _mat->Initialize(i_path, i_aiMat))) {
			// TODO: fail to initialize the material
			return result;
		}
		o_material = _mat;
		printf("Succeed! Loading material: %s.\n", i_path.c_str());
		return result;
	}

	bool cMaterial::InitializeLUA(const std::string& i_path, void*& io_luaState)
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
			const auto luaResult = luaL_loadfile(io_luaState, i_path.c_str());
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
		return result;
	}

	bool cMaterial::ReleaseLUA(void*& io_luaState)
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
				}
				lua_close(io_luaState);
				io_luaState = nullptr;
				result = false;
			}
		}
		else {
			result = false;
			printf("LUA error: Can not close an empty lua table");
			return false;
		}

		return result;
	}

}
