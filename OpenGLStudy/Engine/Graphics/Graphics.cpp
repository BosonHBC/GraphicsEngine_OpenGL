#include <map>
#include "Graphics.h"

#include "Cores/Core.h"
#include "Math/Transform/Transform.h"

#include "Constants/Constants.h"
#include "UniformBuffer/UniformBufferFormats.h"
#include "UniformBuffer/UniformBuffer.h"

namespace Graphics {

	// TODO:
	// Data required to render a frame, right now do not support switching effect(shader)
	struct sDataRequiredToRenderAFrame
	{
		cCamera* CurrentCamera;
		std::vector<std::pair<Graphics::cModel::HANDLE, cTransform*>> ModelToTransform_map;
	};
	// Global data
	// ---------------------------------
	cUniformBuffer s_uniformBuffer_frame(eUniformBufferType::UBT_Frame);
	cUniformBuffer s_uniformBuffer_drawcall(eUniformBufferType::UBT_Drawcall);
	Color s_clearColor;

	sDataRequiredToRenderAFrame s_dataRequiredToRenderAFrame;

	// Effect
	// ---------------------------------
	std::map<const char*, cEffect*> s_KeyToEffect_map;
	cEffect* s_currentEffect;

	// Lighting
	// ---------------------------------
	// There are only one ambient light
	cAmbientLight* s_ambientLight;
	std::vector<cPointLight*> s_pointLight_list;

	bool Initialize()
	{
		auto result = true;

		// Create default effect
		{
			if (!(result = CreateEffect(Constants::CONST_DEFAULT_EFFECT_KEY, Constants::CONST_PATH_DEFAULT_VERTEXSHADER, Constants::CONST_PATH_BLINNPHONG_FRAGMENTSHADER))) {
				printf("Fail to create default effect.\n");
				return result;
			}
			s_currentEffect = GetEffectByKey(Constants::CONST_DEFAULT_EFFECT_KEY);
		}

		// Initialize uniform buffer
		{
			UniformBufferFormats::sFrame _tempFrameData;
			if (!(result = s_uniformBuffer_frame.Initialize(&_tempFrameData))) {
				printf("Fail to initialize uniformBuffer_frame\n");
				return result;
			}
			else {
				// bind frame buffer
				s_uniformBuffer_frame.Bind();
			}
			UniformBufferFormats::sDrawCall _tempDrawData;
			if (!(result = s_uniformBuffer_drawcall.Initialize(&_tempDrawData))) {
				printf("Fail to initialize uniformBuffer_drawcall\n");
				return result;
			}
			else {
				// Bind draw call buffer
				s_uniformBuffer_drawcall.Bind();
			}
		}

		return result;
	}

	void Render()
	{
		// Clear color and poll events
		{
			// clear window
			glClearColor(s_clearColor.r, s_clearColor.g, s_clearColor.b, 1.f);
			// A lot of things can be cleaned like color buffer, depth buffer, so we need to specify what to clear
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glfwPollEvents();
		}
		// Bind effect
		{
			s_currentEffect->UseEffect();
		}

		// Update camera
		{
			if (s_dataRequiredToRenderAFrame.CurrentCamera)
				// Update his location for lighting, TODO: should not update here
				s_dataRequiredToRenderAFrame.CurrentCamera->UpdateUniformLocation(s_currentEffect->GetProgramID());
			else
			{
				printf("No camera in this frame, skip rendering.");
				return;
			}
		}

		// Update frame data
		{
			cCamera* _camera = s_dataRequiredToRenderAFrame.CurrentCamera;

			// 1. Copy frame data
			UniformBufferFormats::sFrame _frame;
			_frame.ViewMatrix = const_cast<glm::f32*>(glm::value_ptr(_camera->GetViewMatrix()));
			_frame.ProjectionMatrix = const_cast<glm::f32*>(glm::value_ptr(_camera->GetProjectionMatrix()));
			// 2. Update frame data
			s_uniformBuffer_frame.Update(&_frame);
		}

		// Update lighting data
		{
			if (s_ambientLight)
				s_ambientLight->Illuminate();
			for (auto it : s_pointLight_list)
			{
				it->Illuminate();
			}
		}

		// Starts draw call
		{
			// loop through every single model
			for (auto it = s_dataRequiredToRenderAFrame.ModelToTransform_map.begin(); it != s_dataRequiredToRenderAFrame.ModelToTransform_map.end(); ++it)
			{
				// 1. Copy draw call data
				UniformBufferFormats::sDrawCall _drawcall;
				_drawcall.ModelMatrix = const_cast<glm::f32*>(glm::value_ptr(it->second->M()));
				_drawcall.NormalMatrix = const_cast<glm::f32*>(glm::value_ptr(glm::transpose(it->second->MInv())));
				// 2. Update draw call data
				s_uniformBuffer_drawcall.Update(&_drawcall);
				// 3. Draw
				cModel* _model = cModel::s_manager.Get(it->first);
				if (_model) {
					_model->Render();
				}
			}
		}
		// clear program
		{
			glUseProgram(0);
		}
		// Swap buffers happens in main rendering loop, not in this render function.
	}

	bool CleanUp()
	{
		auto result = true;
		if (!(result = s_uniformBuffer_frame.CleanUp())) {
			printf("Fail to cleanup uniformBuffer_frame\n");
		}
		if (!(result = s_uniformBuffer_drawcall.CleanUp())) {
			printf("Fail to cleanup uniformBuffer_drawcall\n");
		}

		// Clean up effect
		for (auto it = s_KeyToEffect_map.begin(); it != s_KeyToEffect_map.end(); ++it)
		{
			safe_delete(it->second);
		}
		s_KeyToEffect_map.clear();

		// Clean up point light
		for (auto it = s_pointLight_list.begin(); it != s_pointLight_list.end(); ++it)
		{
			safe_delete(*it);
		}
		s_pointLight_list.clear();

		// Clean up ambient light
		safe_delete(s_ambientLight);

		return result;
	}

	void SubmitDataToBeRendered(cCamera* i_camera, const std::vector<std::pair<Graphics::cModel::HANDLE, cTransform*>>& i_modelToTransform_map)
	{
		s_dataRequiredToRenderAFrame.CurrentCamera = i_camera;
		s_dataRequiredToRenderAFrame.ModelToTransform_map = i_modelToTransform_map;
	}

	bool CreateEffect(const char* i_key, const char* i_vertexShaderPath, const char* i_fragmentShaderPath)
	{
		auto result = true;

		cEffect* newEffect = new  Graphics::cEffect();
		if (!(result = newEffect->CreateProgram(i_vertexShaderPath, i_fragmentShaderPath))) {
			printf("Fail to create default effect.\n");
			safe_delete(newEffect);
			return result;
		}

		s_KeyToEffect_map.insert({ i_key, newEffect });

		return result;
	}

	Graphics::cEffect* GetEffectByKey(const char* i_key)
	{
		if (s_KeyToEffect_map.find(i_key) != s_KeyToEffect_map.end()) {
			return s_KeyToEffect_map.at(i_key);
		}
		return nullptr;
	}

	Graphics::cEffect* GetCurrentEffect()
	{
		return s_currentEffect;
	}

	bool CreateAmbientLight(const Color& i_color, cAmbientLight*& o_ambientLight)
	{
		auto result = true;

		if (result = (s_currentEffect->GetProgramID() == 0)) {
			printf("Can not create ambient light without a valid program id.\n");
			return result;
		}
		if (!(result = (s_ambientLight == nullptr))) {
			printf("Can not create duplicated ambient light.\n");
			return result;
		}
		s_ambientLight = new  cAmbientLight(i_color);
		s_ambientLight->SetupLight(s_currentEffect->GetProgramID(), 0);
		o_ambientLight = s_ambientLight;
		return result;
	}

	bool CreatePointLight(const glm::vec3& i_initialLocation, const Color& i_color, const GLfloat& i_const, const GLfloat& i_linear, const GLfloat& i_quadratic, cPointLight*& o_pointLight)
	{
		auto result = true;
		if (result = (s_currentEffect->GetProgramID() == 0)) {
			printf("Can not create point light without a valid program id.\n");
			return result;
		}
		cPointLight* newPointLight = new cPointLight(i_color, i_const, i_linear, i_quadratic);
		newPointLight->SetLightInitialLocation(i_initialLocation);
		newPointLight->SetupLight(s_currentEffect->GetProgramID(), s_pointLight_list.size());
		o_pointLight = newPointLight;
		s_pointLight_list.push_back(newPointLight);

		// Update point light count
		s_currentEffect->SetPointLightCount(s_pointLight_list.size());
		return result;
	}

}