#include "Blinn/MatBlinn.h"
#include "Assets/LoadTableFromLuaFile.h"
#include "Cubemap/MatCubemap.h"
#include "Externals/ASSIMP_N/include/assimp/scene.h"

namespace Graphics {

	Assets::cAssetManager < cMaterial > cMaterial::s_manager;

	bool cMaterial::Load(const std::string& i_path, cMaterial*& o_material)
	{
		auto result = true;

		cMaterial* _mat = nullptr;
		// Read material type from lua
		eMaterialType _matType = MT_INVALID;
		if (!(result = LoadMaterialTypeInLUA(i_path, _matType))) {
			printf("Material Error: Can not read material type from LUA.\n");
			return result;
		}

		switch (_matType)
		{
		case eMaterialType::MT_INVALID:
			// TODO: invalid material type
			printf("Material Error: Invalid material type.\n");
			result = false;
			return result;
		case eMaterialType::MT_BLINN_PHONG:
			_mat = new (std::nothrow) cMatBlinn();
			break;
		case eMaterialType::MT_CUBEMAP:
			_mat = new (std::nothrow) cMatCubemap();
			break;
		default:
			break;
		}

		if (!(result = _mat)) {
			// Run out of memory
			// TODO: LogError: Out of memory

			return result;
		}

		if (!(result = _mat->Initialize(i_path))) {
			// TODO: fail to initialize the material
			return result;
		}
		o_material = _mat;
		printf("Succeed! Loading material: %s.\n", i_path.c_str());
		return result;
	}

	bool cMaterial::LoadMaterialTypeInLUA(const std::string& i_path, eMaterialType& o_matType)
	{
		auto result = true;
		lua_State* luaState = nullptr;
		//------------------------------
		// Initialize Lua
		//------------------------------
		if (!(result = Assets::InitializeLUA(i_path.c_str(), luaState))) {
			Assets::ReleaseLUA(luaState);
			return result;
		}
		{
			// o_matType
			{
				constexpr auto* const _key = "MaterialType";
				int _tempType = 0;
				if (!(result = Assets::Lua_LoadInteger(luaState, _key, _tempType))) {
					printf("LUA error: fail to load key[%s]", _key);
					return result;
				}
				o_matType = static_cast<eMaterialType>(_tempType);
			}
		}
		//------------------------------
		// Release Lua
		//------------------------------
		result = Assets::ReleaseLUA(luaState);
		return result;
	}

}
