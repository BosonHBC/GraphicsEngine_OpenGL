#pragma once

#include "GL/glew.h"
#include "glfw/glfw3.h"
#include "glm/gtc/type_ptr.hpp"

#include "Color/Color.h"
#include "Effect/Effect.h"
#include "Light/PointLight/PointLight.h"
#include "Light/AmbientLight/AmbientLight.h"
#include "Light/DirectionalLight/DirectionalLight.h"
#include "Camera/Camera.h"
#include "Model/Model.h"

// Graphics stores, initializes, cleans up all data that needs to be rendered
namespace Graphics {

	/** Initialization and clean up function*/
	bool Initialize();

	void ShadowMap_Pass();
	void Render_Pass();

	bool CleanUp();

	void SubmitDataToBeRendered(cCamera* i_camera, const std::vector<std::pair<Graphics::cModel::HANDLE, cTransform*>>& i_modelToTransform_map);

	/** Usage function*/
	bool CreateEffect(const char* i_key, const char* i_vertexShaderPath, const char* i_fragmentShaderPath);
	cEffect* GetEffectByKey(const char* i_key);
	cEffect* GetCurrentEffect();

	/** Lighting related*/
	bool CreateAmbientLight(const Color& i_color, cAmbientLight*& o_ambientLight);
	bool CreatePointLight(const glm::vec3& i_initialLocation,const Color& i_color, const GLfloat& i_const, const GLfloat& i_linear, const GLfloat& i_quadratic, bool i_enableShadow, cPointLight*& o_pointLight);
	bool CreateDirectionalLight(const Color& i_color, glm::vec3 i_direction, bool i_enableShadow, cDirectionalLight*& o_directionalLight);
}