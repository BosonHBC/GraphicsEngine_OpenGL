#pragma once
// Constants.h

#include "Graphics/Color/Color.h"
namespace Constants {
	char const * const CONST_PATH_DEFAULT_COLOR = "Contents/textures/whiteBoard.png";
	char const * const CONST_PATH_DEFAULT_NORMAL = "Contents/textures/DefaultNormal.png";
	char const * const CONST_PATH_SHADER_ROOT = "Contents/shaders/";
	char const * const CONST_PATH_MATERIAL_ROOT = "Contents/materials/";
	char const * const CONST_PATH_TEXTURE_ROOT = "Contents/textures/";
	char const * const CONST_PATH_CUBEMAP_ROOT = "Contents/textures/Cubemaps/";
	char const * const CONST_PATH_MODLE_ROOT = "Contents/models/";
	char const * const CONST_PATH_DEFAULT_VERTEXSHADER = "vertexShader.glsl";
	char const* const CONST_PATH_BLINNPHONG_FRAGMENTSHADER = "BlinnPhong.glsl";

	extern const Color g_arrowColor[3];
	extern const Color g_outlineColor;
}
