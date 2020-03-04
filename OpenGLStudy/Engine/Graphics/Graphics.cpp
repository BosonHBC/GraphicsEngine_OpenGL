#include <map>
#include <stdio.h>
#include <condition_variable> // threading
#include <mutex> // unique_lock
#include "Graphics.h"

#include "Cores/Core.h"
#include "Math/Transform/Transform.h"

#include "Constants/Constants.h"
#include "UniformBuffer/UniformBufferFormats.h"
#include "UniformBuffer/UniformBuffer.h"
#include "FrameBuffer/cFrameBuffer.h"
#include "Application/Application.h"
#include "Application/Window/Window.h"
#include "Material/Blinn/MatBlinn.h"
#include "Material/Unlit/MatUnlit.h"
#include "Time/Time.h"
namespace Graphics {

	// TODO:
	struct sPass
	{
		UniformBufferFormats::sFrame FrameData;
		std::vector<std::pair<Graphics::cModel::HANDLE, cTransform>> ModelToTransform_map;
		void(*RenderPassFunction) ();
		sPass() {}
	};
	// Data required to render a frame, right now do not support switching effect(shader)
	struct sDataRequiredToRenderAFrame
	{
		UniformBufferFormats::sClipPlane s_ClipPlane;
		std::vector<sPass> s_renderPasses;
		// Lighting data
		std::vector<cPointLight> s_pointLights;
		std::vector<cSpotLight> s_spotLights;
		cAmbientLight s_ambientLight;
		cDirectionalLight s_directionalLight;
	};
	unsigned int s_currentRenderPass = 0;
	// Global data
	// ---------------------------------
	cUniformBuffer s_uniformBuffer_frame(eUniformBufferType::UBT_Frame);
	cUniformBuffer s_uniformBuffer_drawcall(eUniformBufferType::UBT_Drawcall);
	cUniformBuffer s_uniformBuffer_Lighting(eUniformBufferType::UBT_Lighting);
	cUniformBuffer s_uniformBuffer_ClipPlane(eUniformBufferType::UBT_ClipPlane);

	UniformBufferFormats::sLighting s_globalLightingData;

	Color s_clearColor;

	// This buffer capture the camera view
	cFrameBuffer s_cameraCapture;

	sDataRequiredToRenderAFrame s_dataRequiredToRenderAFrame[2];
	auto* s_dateSubmittedByApplicationThread = &s_dataRequiredToRenderAFrame[0];
	auto* s_dateRenderingByGraphicThread = &s_dataRequiredToRenderAFrame[1];

	// Effect
	// ---------------------------------
	std::map<const char*, cEffect*> s_KeyToEffect_map;
	cEffect* s_currentEffect;

	// Lighting
	// ---------------------------------
	// There are only one ambient and directional light
	cAmbientLight* s_ambientLight;
	cDirectionalLight* s_directionalLight;
	std::vector<cPointLight*> s_pointLight_list;
	std::vector<cSpotLight*> s_spotLight_list;

	// Transform hint
	Graphics::cModel::HANDLE s_arrow;
	Color s_arrowColor[3] = { Color(0, 0, 0.8f), Color(0.8f, 0, 0),Color(0, 0.8f, 0) };

	// Threading
	std::condition_variable s_whenAllDataHasBeenSubmittedFromApplicationThread;
	std::condition_variable s_whenDataHasBeenSwappedInGraphicThread;
	std::mutex s_graphicMutex;
	//functions
	void RenderScene();
	void RenderScene_shadowMap();

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
			s_currentEffect->FixSamplerError();
		}
		// Create shadow map effect
		{
			if (!(result = CreateEffect("ShadowMap",
				"shadowmaps/directionalShadowMap/directional_shadow_map_vert.glsl",
				"shadowmaps/directionalShadowMap/directional_shadow_map_frag.glsl"))) {
				printf("Fail to create shadow map effect.\n");
				return result;
			}
		}
		// Create cube map effect
		{
			if (!(result = CreateEffect("CubemapEffect",
				"cubemap/cubemap_vert.glsl",
				"cubemap/cubemap_frag.glsl"))) {
				printf("Fail to create cube map effect.\n");
				return result;
			}
		}
		// Create unlit effect
		{
			if (!(result = CreateEffect("UnlitEffect",
				"unlit/arrow_vert.glsl",
				"unlit/arrow_frag.glsl"))) {
				printf("Fail to create unlit effect.\n");
				return result;
			}
		}
		// Initialize uniform buffer
		{
			// Frame buffer
			if (result = s_uniformBuffer_frame.Initialize(nullptr)) {
				s_uniformBuffer_frame.Bind();
			}
			else {
				printf("Fail to initialize uniformBuffer_frame\n");
				return result;
			}
			// draw call
			if (result = s_uniformBuffer_drawcall.Initialize(nullptr)) {
				s_uniformBuffer_drawcall.Bind();
			}
			else {
				printf("Fail to initialize uniformBuffer_drawcall\n");
				return result;
			}
			if (result = s_uniformBuffer_Lighting.Initialize(nullptr)) {
				s_uniformBuffer_Lighting.Bind();
			}
			else
			{
				printf("Fail to initialize uniformBuffer_Lighting\n");
				return result;
			}
			if (result = s_uniformBuffer_ClipPlane.Initialize(nullptr))
			{
				s_uniformBuffer_ClipPlane.Bind();
			}
			else {
				printf("Fail to initialize uniformBuffer_ClipPlane\n");
				return result;
			}
		}


		if (!(result = s_cameraCapture.Initialize(800, 600, ETT_FRAMEBUFFER_COLOR))) {
			printf("Fail to create camera capture frame buffer.\n");
			return result;
		}

		for (auto it : s_KeyToEffect_map)
		{
			if (!(result = it.second->ValidateProgram()))
			{
				return result;
			}
		}
		// Load arrows
		{
			std::string _arrowPath = "Contents/models/arrow.model";
			if (!(result = Graphics::cModel::s_manager.Load(_arrowPath, s_arrow)))
			{
				printf("Failed to Load arrow_forward!\n");
				return result;
			}
		}


		return result;
	}

	void RenderFrame()
	{
		/** 1. Wait for data being submitted here */ 
		// Acquire the lock
		std::unique_lock<std::mutex> _mlock(s_graphicMutex);
		// Wait until the conditional variable is signaled
		s_whenAllDataHasBeenSubmittedFromApplicationThread.wait(_mlock);

		// After data has been submitted, swap the data
		std::swap(s_dateSubmittedByApplicationThread, s_dateRenderingByGraphicThread);
		s_whenDataHasBeenSwappedInGraphicThread.notify_one();

		/** 2. Start to render pass one by one */
		for (size_t i = 0; i < s_dateRenderingByGraphicThread->s_renderPasses.size(); ++i)
		{
			// Update frame data
			s_uniformBuffer_ClipPlane.Update(&s_dateRenderingByGraphicThread->s_ClipPlane);
			s_uniformBuffer_frame.Update(&s_dateRenderingByGraphicThread->s_renderPasses[i].FrameData);
			// Update current render pass
			s_currentRenderPass = i;
			// Execute pass function
			s_dateRenderingByGraphicThread->s_renderPasses[i].RenderPassFunction();
		}
	}

	void DirectionalShadowMap_Pass()
	{
		s_currentEffect = GetEffectByKey("ShadowMap");
		s_currentEffect->UseEffect();

		cFrameBuffer* _directionalLightFBO = s_directionalLight->GetShadowMap();
		if (_directionalLightFBO && s_directionalLight->IsShadowEnabled()) {

			{
				Application::cApplication* _app = Application::GetCurrentApplication();
				if (_app) {
					_app->GetCurrentWindow()->SetViewportSize(_directionalLightFBO->GetWidth(), _directionalLightFBO->GetHeight());
				}
			}
			// write buffer to the texture
			_directionalLightFBO->Write();

			glClearColor(0, 0, 0, 1.f);
			glClear(/*GL_COLOR_BUFFER_BIT | */GL_DEPTH_BUFFER_BIT);

			// Draw scenes
			RenderScene_shadowMap();

			// switch back to original buffer
			_directionalLightFBO->UnWrite();
			assert(glGetError() == GL_NO_ERROR);
		}
		s_currentEffect->UnUseEffect();
	}

	void SpotLightShadowMap_Pass()
	{
		s_currentEffect = GetEffectByKey("ShadowMap");
		s_currentEffect->UseEffect();

		for (auto i = 0; i < s_spotLight_list.size(); ++i)
		{
			cFrameBuffer* _spotLightFB = s_spotLight_list[i]->GetShadowMap();
			if (_spotLightFB) {

				{
					Application::cApplication* _app = Application::GetCurrentApplication();
					if (_app) {
						_app->GetCurrentWindow()->SetViewportSize(_spotLightFB->GetWidth(), _spotLightFB->GetHeight());
					}
				}
				// write buffer to the texture
				_spotLightFB->Write();
				assert(glGetError() == GL_NO_ERROR);
				glClearColor(0, 0, 0, 1.f);
				glClear(GL_DEPTH_BUFFER_BIT);

				// Draw scenes
				RenderScene_shadowMap();

				// switch back to original buffer
				_spotLightFB->UnWrite();
				assert(glGetError() == GL_NO_ERROR);
			}
		}

		s_currentEffect->UnUseEffect();
	}

	void Render_Pass_CaptureCameraView()
	{
		// Enable clip plane0
		{
			glEnable(GL_CLIP_PLANE0);
		}
		// Bind effect
		{
			s_currentEffect = GetEffectByKey(Constants::CONST_DEFAULT_EFFECT_KEY);
			s_currentEffect->UseEffect();
			if (s_directionalLight)
				s_directionalLight->SetupLight(s_currentEffect->GetProgramID(), 0);

		}
		// Reset window size
		{
			Application::cApplication* _app = Application::GetCurrentApplication();
			if (_app) {
				_app->GetCurrentWindow()->SetViewportSize(s_cameraCapture.GetWidth(), s_cameraCapture.GetHeight());
			}
		}
		s_cameraCapture.Write();

		// Clear color and buffers
		{
			// clear window
			glClearColor(0, 0, 0, 1.f);
			// A lot of things can be cleaned like color buffer, depth buffer, so we need to specify what to clear
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		}

		// Update lighting data
		{

			if (s_ambientLight)
				s_ambientLight->Illuminate();

			if (s_directionalLight) {
				s_directionalLight->Illuminate();
			}

			for (auto it : s_pointLight_list)
			{
				it->Illuminate();
			}
			for (auto it : s_spotLight_list)
			{
				it->Illuminate();
			}
			s_globalLightingData.pointLightCount = s_pointLight_list.size();
			s_globalLightingData.spotLightCount = s_spotLight_list.size();
			s_uniformBuffer_Lighting.Update(&s_globalLightingData);
		}
		// Start a draw call loop
		RenderScene();

		s_cameraCapture.UnWrite();
		// clear program
		{
			s_currentEffect->UnUseEffect();
		}
		// Enable clip plane0
		{
			glDisable(GL_CLIP_PLANE0);
		}
	}

	Graphics::cFrameBuffer* GetCameraCaptureFrameBuffer()
	{
		return &s_cameraCapture;
	}

	void SubmitClipPlaneData(const glm::vec4& i_plane0, const glm::vec4& i_plane1 /*= glm::vec4(0,0,0,0)*/, const glm::vec4& i_plane2 /*= glm::vec4(0, 0, 0, 0)*/, const glm::vec4& i_plane3 /*= glm::vec4(0, 0, 0, 0)*/)
	{
		s_dateSubmittedByApplicationThread->s_ClipPlane = UniformBufferFormats::sClipPlane(i_plane0, i_plane1, i_plane2, i_plane3);
	}

	void SubmitLightingData(const std::vector<cPointLight>& i_pointLights, const std::vector<cSpotLight>& i_spotLights, const cAmbientLight& i_ambientLight, const cDirectionalLight& i_directionalLight)
	{

	}
	void SubmitDataToBeRendered(const UniformBufferFormats::sFrame& i_frameData, const std::vector<std::pair<Graphics::cModel::HANDLE, cTransform>>& i_modelToTransform_map, void(*func_ptr)())
	{
		sPass inComingPasses;
		inComingPasses.FrameData = i_frameData;
		inComingPasses.ModelToTransform_map = i_modelToTransform_map;
		inComingPasses.RenderPassFunction = func_ptr;
		s_dateSubmittedByApplicationThread->s_renderPasses.push_back(inComingPasses);
	}

	void SubmitTransformToBeDisplayedWithTransformGizmo(const std::vector< cTransform*>& i_transforms)
	{
		glDisable(GL_DEPTH_TEST);
		s_currentEffect = GetEffectByKey("UnlitEffect");
		s_currentEffect->UseEffect();

		// TODE: Error
		//s_uniformBuffer_frame.Update(&s_dataRequiredToRenderAFrame.FrameData);

		for (auto it = i_transforms.begin(); it != i_transforms.end(); ++it)
		{
			// Get forward transform
			cTransform arrowTransform[3];
			{
				// Forward
				arrowTransform[0].SetRotation((*it)->Rotation() * glm::quat(glm::vec3(glm::radians(90.f), 0, 0)));
				// Right
				arrowTransform[1].SetRotation((*it)->Rotation() * glm::quat(glm::vec3(0, 0, glm::radians(90.f))));
				// Up
				arrowTransform[2].SetRotation((*it)->Rotation() * glm::quat(glm::vec3(0, glm::radians(90.f), 0)));
			}

			cModel* _model = cModel::s_manager.Get(s_arrow);
			cMatUnlit* _arrowMat = dynamic_cast<cMatUnlit*>(_model->GetMaterialAt());

			for (int i = 0; i < 3; ++i)
			{
				arrowTransform[i].SetPosition((*it)->Position());
				arrowTransform[i].SetScale(glm::vec3(2, 10, 2));
				arrowTransform[i].Update();
				s_uniformBuffer_drawcall.Update(&UniformBufferFormats::sDrawCall(arrowTransform[i].M(), arrowTransform[i].TranspostInverse()));

				if (_model) {
					_arrowMat->SetUnlitColor(s_arrowColor[i]);
					_model->UpdateUniformVariables(s_currentEffect->GetProgramID());
					_model->Render();
				}
			}
		}


		s_currentEffect->UnUseEffect();
		glEnable(GL_DEPTH_TEST);
	}

	void ClearApplicationThreadData()
	{
		s_dateSubmittedByApplicationThread->s_renderPasses.clear();
		s_dateSubmittedByApplicationThread->s_pointLights.clear();
		s_dateSubmittedByApplicationThread->s_spotLights.clear();

	}
	void Render_Pass()
	{

		// Bind effect
		{
			s_currentEffect = GetEffectByKey(Constants::CONST_DEFAULT_EFFECT_KEY);
			s_currentEffect->UseEffect();
			if (s_directionalLight)
				s_directionalLight->SetupLight(s_currentEffect->GetProgramID(), 0);
		}
		// Reset window size
		{
			Application::cApplication* _app = Application::GetCurrentApplication();
			if (_app) {
				_app->GetCurrentWindow()->SetViewportSize(_app->GetCurrentWindow()->GetBufferWidth(), _app->GetCurrentWindow()->GetBufferHeight());
			}
		}
		// Clear color and buffers
		{
			// clear window
			glClearColor(s_clearColor.r, s_clearColor.g, s_clearColor.b, 1.f);
			// A lot of things can be cleaned like color buffer, depth buffer, so we need to specify what to clear
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		}

		// Update lighting data
		{
			if (s_ambientLight)
				s_ambientLight->Illuminate();

			if (s_directionalLight) {
				s_directionalLight->Illuminate();
				s_directionalLight->SetLightUniformTransform();
				if (s_directionalLight->IsShadowEnabled()) {
					s_directionalLight->UseShadowMap(2);
					s_directionalLight->GetShadowMap()->Read(GL_TEXTURE2);
				}
			}

			for (auto it : s_pointLight_list)
			{
				it->Illuminate();
			}
			for (auto it : s_spotLight_list)
			{
				it->Illuminate();
				it->SetupLight(s_currentEffect->GetProgramID(), 0);
				it->SetLightUniformTransform();
				if (it->IsShadowEnabled()) {
					it->UseShadowMap(5);
					it->GetShadowMap()->Read(GL_TEXTURE5);
				}
			}
			s_globalLightingData.pointLightCount = s_pointLight_list.size();
			s_globalLightingData.spotLightCount = s_spotLight_list.size();
			s_uniformBuffer_Lighting.Update(&s_globalLightingData);

		}
		// Start a draw call loop
		RenderScene();
		// clear program
		{
			s_currentEffect->UnUseEffect();
		}
		// Swap buffers happens in main rendering loop, not in this render function.
	}

	void CubeMap_Pass()
	{
		// change depth function so depth test passes when values are equal to depth buffer's content
		glDepthFunc(GL_LEQUAL);

		s_currentEffect = GetEffectByKey("CubemapEffect");
		s_currentEffect->UseEffect();

		for (auto it = s_dateRenderingByGraphicThread->s_renderPasses[s_currentRenderPass].ModelToTransform_map.begin();
			it != s_dateRenderingByGraphicThread->s_renderPasses[s_currentRenderPass].ModelToTransform_map.end(); ++it)
		{
			// 1. Do not need to update draw call data because in cubemap.vert, there is no model matrix and normal matrix
			// 2. Draw
			cModel* _model = cModel::s_manager.Get(it->first);
			if (_model)
			{
				_model->Render();
			}
		}

		s_currentEffect->UnUseEffect();
		// set depth function back to default
		glDepthFunc(GL_LESS);
	}

	void RenderScene_shadowMap()
	{
		// loop through every single model
		for (auto it = s_dateRenderingByGraphicThread->s_renderPasses[s_currentRenderPass].ModelToTransform_map.begin();
			it != s_dateRenderingByGraphicThread->s_renderPasses[s_currentRenderPass].ModelToTransform_map.end(); ++it)
		{
			// 1. Update draw call data
			s_uniformBuffer_drawcall.Update(&UniformBufferFormats::sDrawCall(it->second.M(), it->second.TranspostInverse()));
			// 2. Draw
			cModel* _model = cModel::s_manager.Get(it->first);
			if (_model) {
				_model->RenderWithoutMaterial();
			}
		}

	}
	void RenderScene()
	{
		// loop through every single model
		for (auto it = s_dateRenderingByGraphicThread->s_renderPasses[s_currentRenderPass].ModelToTransform_map.begin();
			it != s_dateRenderingByGraphicThread->s_renderPasses[s_currentRenderPass].ModelToTransform_map.end();
			++it)
		{
			// 1. Update draw call data
			s_uniformBuffer_drawcall.Update(&UniformBufferFormats::sDrawCall(it->second.M(), it->second.TranspostInverse()));
			// 2. Draw
			cModel* _model = cModel::s_manager.Get(it->first);
			if (_model) {
				_model->Render();
			}
		}

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
		if (!(result = cMatBlinn::GetUniformBuffer().CleanUp())) {
			printf("Fail to cleanup uniformBuffer_MatBlinnPhong\n");
		}
		if (!(result = s_uniformBuffer_Lighting.CleanUp())) {
			printf("Fail to cleanup uniformBuffer_Lighting\n");
		}
		if (!(result = s_uniformBuffer_ClipPlane.CleanUp())) {
			printf("Fail to cleanup uniformBuffer_ClipPlane\n");
		}
		Graphics::cModel::s_manager.Release(s_arrow);
		// Clean up effect
		for (auto it = s_KeyToEffect_map.begin(); it != s_KeyToEffect_map.end(); ++it)
		{
			safe_delete(it->second);
		}
		s_KeyToEffect_map.clear();

		// Clean up point light
		for (auto it = s_pointLight_list.begin(); it != s_pointLight_list.end(); ++it)
		{
			(*it)->CleanUpShadowMap();
			safe_delete(*it);
		}
		s_pointLight_list.clear();
		for (auto it = s_spotLight_list.begin(); it != s_spotLight_list.end(); ++it)
		{
			(*it)->CleanUpShadowMap();
			safe_delete(*it);
		}
		s_spotLight_list.clear();


		// Clean up ambient light
		s_ambientLight->CleanUpShadowMap();
		safe_delete(s_ambientLight);
		s_directionalLight->CleanUpShadowMap();
		safe_delete(s_directionalLight);

		s_cameraCapture.~cFrameBuffer();

		return result;
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

	//----------------------------------------------------------------------------------
	/** Lighting related */
	//----------------------------------------------------------------------------------

	Graphics::UniformBufferFormats::sLighting& GetGlobalLightingData()
	{
		return s_globalLightingData;
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

	bool CreatePointLight(const glm::vec3& i_initialLocation, const Color& i_color, const GLfloat& i_const, const GLfloat& i_linear, const GLfloat& i_quadratic, bool i_enableShadow, cPointLight*& o_pointLight)
	{
		auto result = true;
		if (result = (s_currentEffect->GetProgramID() == 0)) {
			printf("Can not create point light without a valid program id.\n");
			return result;
		}
		// TODO: lighting, range should be passed in
		cPointLight* newPointLight = new cPointLight(i_color, i_initialLocation, 300.f, i_const, i_linear, i_quadratic);
		newPointLight->SetupLight(s_currentEffect->GetProgramID(), s_pointLight_list.size());
		newPointLight->SetEnableShadow(i_enableShadow);
		o_pointLight = newPointLight;
		s_pointLight_list.push_back(newPointLight);


		return result;
	}

	bool CreateSpotLight(const glm::vec3& i_initialLocation, const glm::vec3& i_direction, const Color& i_color, const GLfloat& i_edge, const GLfloat& i_const, const GLfloat& i_linear, const GLfloat& i_quadratic, bool i_enableShadow, cSpotLight*& o_spotLight)
	{
		auto result = true;
		if (result = (s_currentEffect->GetProgramID() == 0)) {
			printf("Can not create spot light without a valid program id.\n");
			return result;
		}
		cSpotLight* newSpotLight = new cSpotLight(i_color, i_initialLocation, glm::normalize(i_direction), i_edge, 300.f, i_const, i_linear, i_quadratic);
		newSpotLight->SetupLight(s_currentEffect->GetProgramID(), s_spotLight_list.size());
		newSpotLight->SetEnableShadow(i_enableShadow);
		newSpotLight->CreateShadowMap(2048, 2048);
		o_spotLight = newSpotLight;
		s_spotLight_list.push_back(newSpotLight);

		return result;
	}

	bool CreateDirectionalLight(const Color& i_color, glm::vec3 i_direction, bool i_enableShadow, cDirectionalLight*& o_directionalLight)
	{
		auto result = true;
		if (result = (s_currentEffect->GetProgramID() == 0)) {
			printf("Can not create directional light without a valid program id.\n");
			return result;
		}
		cDirectionalLight* newDirectionalLight = new cDirectionalLight(i_color, glm::normalize(i_direction));
		newDirectionalLight->SetupLight(s_currentEffect->GetProgramID(), 0);
		newDirectionalLight->SetEnableShadow(i_enableShadow);
		newDirectionalLight->CreateShadowMap(2048, 2048);


		o_directionalLight = newDirectionalLight;
		s_directionalLight = newDirectionalLight;
		return result;
	}

	//----------------------------------------------------------------------------------
	/** Threading related */
	//----------------------------------------------------------------------------------
	void Notify_DataHasBeenSubmited()
	{
		s_whenAllDataHasBeenSubmittedFromApplicationThread.notify_one();
	}

	void MakeApplicationThreadWaitForSwapingData(std::mutex& i_applicationMutex)
	{
		std::unique_lock<std::mutex> lck(i_applicationMutex);
		constexpr unsigned int timeToWait_inMilliseconds = 10;
		s_whenDataHasBeenSwappedInGraphicThread.wait_for(lck, std::chrono::milliseconds(timeToWait_inMilliseconds));
	}

}