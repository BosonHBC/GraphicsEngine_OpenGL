#include "Blinn/MatBlinn.h"
#include "assimp/scene.h"

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

}
