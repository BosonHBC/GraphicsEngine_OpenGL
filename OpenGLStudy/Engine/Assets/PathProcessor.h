#pragma once

/*
*/
#ifndef ASSET_PATH_DEFINED
#define ASSET_PATH_DEFINED
#include "string"
#include "stdio.h"
#include "assert.h"
#include "Constants/Constants.h"
#include "Graphics/Material/Material.h"
#include "Graphics/Texture/Texture.h"
#include "Graphics/Model/Model.h"

namespace Assets {
	namespace Path {
		template<class T>
		std::string ProcessPath(const std::string& i_path)
		{
			printf("Invalid class type to process. Specify correct type.");
			assert(false);
			return i_path;
		}

		template<>
		std::string ProcessPath<Graphics::cMaterial>(const std::string& i_path)
		{
			std::string _tempstr = i_path;
			_tempstr.insert(0, Constants::CONST_PATH_MATERIAL_ROOT);
			return _tempstr;
		}
		template<>
		std::string ProcessPath<Graphics::cTexture>(const std::string& i_path)
		{
			std::string _tempstr = i_path;
			_tempstr.insert(0, Constants::CONST_PATH_TEXTURE_ROOT);
			return _tempstr;
		}
	}
}
#endif // !ASSET_PATH_DEFINED

