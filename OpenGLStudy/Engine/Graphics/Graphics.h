#pragma once

#include "GL/glew.h"
#include "glfw/glfw3.h"
#include "glm/gtc/type_ptr.hpp"

#include "Color/Color.h"
#include "Effect/Effect.h"
#include "Camera/EditorCamera/EditorCamera.h"
// Graphics stores, initializes, cleans up all data that needs to be rendered
namespace Graphics {

	/** Initialization and clean up function*/
	bool Initialize();
	void Render();
	bool CleanUp();

	/** Usage function*/
	bool CreateEffect(const char* i_key, const char* i_vertexShaderPath, const char* i_fragmentShaderPath);
	cEffect* GetEffectByKey(const char* i_key);

	bool CreateAmbientLight(const Color& i_color);
	bool CreatePointLight(const glm::vec3& i_initialLocation,const Color& i_color, const GLfloat& i_const, const GLfloat& i_linear, const GLfloat& i_quadratic);
}