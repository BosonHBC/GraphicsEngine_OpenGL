#include "PathProcessor.h"
#include "Constants/Constants.h"

namespace Assets {

	std::string ProcessPathMat(const std::string& i_path)
	{
		std::string _tempstr = i_path;
		_tempstr.insert(0, Constants::CONST_PATH_MATERIAL_ROOT);
		return _tempstr;
	}

	std::string ProcessPathTex(const std::string& i_path)
	{
		std::string _tempstr = i_path;
		_tempstr.insert(0, Constants::CONST_PATH_TEXTURE_ROOT);
		return _tempstr;
	}

}