#include "Blinn/MatBlinn.h"

namespace Graphics {
	bool cMaterial::Load(const std::string& i_path, cMaterial*& o_material)
	{
		auto result = true;

		cMaterial* _mat = nullptr;
		// TODO: read the material type from LUA file
		eMaterialType _matType = MT_BLINN_PHONG;

		switch (_matType)
		{
		case Graphics::cMaterial::MT_INVALID:
			_mat = new (std::nothrow) cMaterial(_matType);
			break;
		case Graphics::cMaterial::MT_BLINN_PHONG:
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
		else
		{
		}
	}

}
