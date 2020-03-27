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
#include "Material/PBR_MR/MatPBRMR.h"
#include "Time/Time.h"
#include "Assets/PathProcessor.h"
#include "EnvironmentCaptureManager.h" 

#include "Application/Window/WindowInput.h"

namespace Graphics {

	unsigned int s_currentRenderPass = 0;
	sDataRequiredToRenderAFrame s_dataRequiredToRenderAFrame[2];
	auto* s_dataSubmittedByApplicationThread = &s_dataRequiredToRenderAFrame[0];
	auto* s_dataRenderingByGraphicThread = &s_dataRequiredToRenderAFrame[1];

	// Threading
	std::condition_variable s_whenAllDataHasBeenSubmittedFromApplicationThread;
	std::condition_variable s_whenDataHasBeenSwappedInRenderThread;
	std::mutex s_graphicMutex;

	// Global data
	// ------------------------------------------------------------------------------------------------------------------------------------
	cUniformBuffer s_uniformBuffer_frame(eUniformBufferType::UBT_Frame);
	cUniformBuffer s_uniformBuffer_drawcall(eUniformBufferType::UBT_Drawcall);
	cUniformBuffer s_uniformBuffer_Lighting(eUniformBufferType::UBT_Lighting);
	cUniformBuffer s_uniformBuffer_ClipPlane(eUniformBufferType::UBT_ClipPlane);

	// Pre-defined mesh & textures
	cModel::HANDLE s_cubeHandle;
	cModel::HANDLE s_arrowHandle;
	cModel::HANDLE s_quadHandle;
	cMesh::HANDLE s_point;
	cTexture::HANDLE s_spruitSunRise_HDR;
	cTexture::HANDLE g_quadTeapotDisplacementMapHandle;
	// Lighting data
	UniformBufferFormats::sLighting s_globalLightingData;

	// clear color
	Color s_clearColor;
	// arrow colors
	Color s_arrowColor[3] = { Color(0, 0, 0.8f), Color(0.8f, 0, 0),Color(0, 0.8f, 0) };

	// Frame buffers
	// ------------------------------------------------------------------------------------------------------------------------------------
	// This buffer capture the camera view
	cFrameBuffer s_cameraCapture;
	// Rectangular HDR map to cubemap
	cEnvProbe s_cubemapProbe;
	// the brdfLUTTexture for integrating the brdf
	cFrameBuffer s_brdfLUTTexture;

	// Effect
	// ------------------------------------------------------------------------------------------------------------------------------------
	std::map<eEffectType, cEffect*> s_KeyToEffect_map;
	cEffect* s_currentEffect;

	// Lighting
	// ------------------------------------------------------------------------------------------------------------------------------------
	// There are only one ambient and directional light
	cAmbientLight* s_ambientLight;
	cDirectionalLight* s_directionalLight;
	std::vector<cPointLight*> s_pointLight_list;
	std::vector<cSpotLight*> s_spotLight_list;

	// spot light first
#define SHADOWMAP_START_TEXTURE_UNIT 13

	// Functions
	// ------------------------------------------------------------------------------------------------------------------------------------
	void RenderScene(GLenum i_drawMode = GL_TRIANGLES);
	void RenderSceneWithoutMaterial();
	void Gizmo_DrawDebugCaptureVolume();

	void FixSamplerProblem(const eEffectType& i_effectKey)
	{
		// Fix sampler problem before validating the program

		cEffect* _effect = GetEffectByKey(i_effectKey);
		_effect->UseEffect();
		GLuint _programID = _effect->GetProgramID();
		char _charBuffer[64] = { '\0' };

		_effect->SetInteger("BrdfLUTMap", 4);

		const auto maxCubemapMixing = EnvironmentCaptureManager::MaximumCubemapMixingCount();
		constexpr auto cubemapStartID = 5;
		for (size_t i = 0; i < maxCubemapMixing; ++i)
		{
			snprintf(_charBuffer, sizeof(_charBuffer), "IrradianceMap[%d]", i);
			_effect->SetInteger(_charBuffer, cubemapStartID + i);
			snprintf(_charBuffer, sizeof(_charBuffer), "PrefilterMap[%d]", i);
			_effect->SetInteger(_charBuffer, cubemapStartID + maxCubemapMixing + i);
		}
		for (int i = 0; i < MAX_COUNT_PER_LIGHT; ++i)
		{
			snprintf(_charBuffer, sizeof(_charBuffer), "spotlightShadowMap[%d]", i);
			_effect->SetInteger(_charBuffer, SHADOWMAP_START_TEXTURE_UNIT + i);
			snprintf(_charBuffer, sizeof(_charBuffer), "pointLightShadowMap[%d]", i);
			_effect->SetInteger(_charBuffer, SHADOWMAP_START_TEXTURE_UNIT + MAX_COUNT_PER_LIGHT + i);
		}
		_effect->SetInteger("directionalShadowMap", SHADOWMAP_START_TEXTURE_UNIT + MAX_COUNT_PER_LIGHT * 2);
		assert(GL_NO_ERROR == glGetError());
		_effect->UnUseEffect();
	}

	bool Initialize()
	{
		auto result = true;
		// Initialize sub-modules
		{
			if (!(result = EnvironmentCaptureManager::Initialize()))
			{
				printf("Fail to initialize environment capture manager.\n");
				return result;
			}
		}
		// Create effects
		{
			// Create BlinnPhong effect
			{
				if (!(result = CreateEffect(EET_BlinnPhong, Constants::CONST_PATH_DEFAULT_VERTEXSHADER, Constants::CONST_PATH_BLINNPHONG_FRAGMENTSHADER))) {
					printf("Fail to create default effect.\n");
					return result;
				}
				s_currentEffect = GetEffectByKey(EET_BlinnPhong);
				FixSamplerProblem(EET_BlinnPhong);
			}
			// Create shadow map effect
			{
				if (!(result = CreateEffect(EET_ShadowMap,
					"shadowmaps/directionalShadowMap/directional_shadow_map_vert.glsl",
					"shadowmaps/directionalShadowMap/directional_shadow_map_frag.glsl"))) {
					printf("Fail to create shadow map effect.\n");
					return result;
				}
			}
			// Create OmniShadowmap display effect
			{
				if (!(result = CreateEffect(EET_OmniShadowMap,
					"shadowmaps/omniShadowMap/omni_shadow_map_vert.glsl",
					"shadowmaps/omniShadowMap/omni_shadow_map_frag.glsl",
					"shadowmaps/omniShadowMap/omni_shadow_map_geom.glsl"
				))) {
					printf("Fail to create OmniShadowMap effect.\n");
					return result;
				}
			}
			// Create cube map effect
			{
				if (!(result = CreateEffect(EET_Cubemap,
					"cubemap/cubemap_vert.glsl",
					"cubemap/cubemap_frag.glsl"))) {
					printf("Fail to create cube map effect.\n");
					return result;
				}
			}
			// Create unlit effect
			{
				if (!(result = CreateEffect(EET_Unlit,
					"unlit/arrow_vert.glsl",
					"unlit/arrow_frag.glsl"))) {
					printf("Fail to create unlit effect.\n");
					return result;
				}
			}
			// Create normal display effect
			{
				if (!(result = CreateEffect(EET_NormalDisplay,
					"normalDisplayer/normal_vert.glsl",
					"normalDisplayer/normal_frag.glsl",
					"normalDisplayer/normal_geom.glsl"
				))) {
					printf("Fail to create normal display effect.\n");
					return result;
				}
			}
			// Create PBR_MetallicRoughness effect
			{
				if (!(result = CreateEffect(EET_PBR_MR,
					"vertexShader.glsl",
					"PBR_MetallicRoughness.glsl"
				))) {
					printf("Fail to create PBR_MR effect.\n");
					return result;
				}
				FixSamplerProblem(EET_PBR_MR);
			}
			// Create rectangular HDR map to cubemap effect
			{
				if (!(result = CreateEffect(EET_HDRToCubemap,
					"equirectangularToCubemap/rect_to_cube_vert.glsl",
					"equirectangularToCubemap/rect_to_cube_frag.glsl"))) {
					printf("Fail to create RectToCubemap effect.\n");
					return result;
				}
			}
			// Crete diffuse irradiance convolution effect
			{
				if (!(result = CreateEffect(EET_IrradConvolution,
					"cubemap/cubemap_vert.glsl",
					"cubemap/cubemap_diff_irrad_convolution_frag.glsl"))) {
					printf("Fail to create IrradConvolution effect.\n");
					return result;
				}
			}
			// Crete cube map pre-filtering effect
			{
				if (!(result = CreateEffect(EET_CubemapPrefilter,
					"cubemap/cubemap_vert.glsl",
					"cubemap/cubemap_spec_prefilter_frag.glsl"))) {
					printf("Fail to create CubemapPrefilter effect.\n");
					return result;
				}
			}
			// Crete BRDF integration effect
			{
				if (!(result = CreateEffect(EET_BrdfIntegration,
					"IntegrateBRDF/intergrate_brdf_vert.glsl",
					"IntegrateBRDF/intergrate_brdf_frag.glsl"))) {
					printf("Fail to create BRDF Integration effect.\n");
					return result;
				}
			}
			// Create draw debug circle
			{
				if (!(result = CreateEffect(EET_DrawDebugCircles,
					"drawDebugCircles/debugCircle_vert.glsl",
					"drawDebugCircles/debugCircle_frag.glsl",
					"drawDebugCircles/debugCircle_geom.glsl"
				))) {
					printf("Fail to create OmniShadowMap effect.\n");
					return result;
				}
			}
			// Tess quad effect
			{
				if (!(result = CreateEffect(EET_TessQuad,
					"tessellation/tess_quad_vert.glsl",
					"PBR_MetallicRoughness.glsl",
					"",
					"tessellation/tess_quad_ctrl.glsl",
					"tessellation/tess_quad_evalue.glsl"
				))) {
					printf("Fail to create TessQuad effect.\n");
					return result;
				}

				FixSamplerProblem(EET_TessQuad);
			}
			// Create triangulation display effect
			{
				if (!(result = CreateEffect(EET_TriangulationDisplay,
					"tessellation/tess_quad_vert.glsl",
					"normalDisplayer/normal_frag.glsl",
					"triangulationDisplayer/triangulation_geom.glsl",
					"tessellation/tess_quad_ctrl.glsl",
					"tessellation/tess_quad_evalue.glsl"
				))) {
					printf("Fail to create TriangulationDisplay effect.\n");
					return result;
				}
			}
			
			// validate all programs
			for (auto it : s_KeyToEffect_map)
			{
				it.second->ValidateProgram();
			}
		}

		// Initialize uniform buffer
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

		if (!(result = s_cameraCapture.Initialize(800, 600, ETT_FRAMEBUFFER_PLANNER_REFLECTION))) {
			printf("Fail to create camera capture frame buffer.\n");
			return result;
		}
		assert(GL_NO_ERROR == glGetError());
		// Initialize environment probes
		{
			// This is for changing rect hdr map to cubemap
			if (!(result = s_cubemapProbe.Initialize(10, 2048, 2048, ETT_FRAMEBUFFER_HDR_CUBEMAP))) {
				printf("Fail to create cubemap probe.\n");
				return result;
			}

			constexpr GLuint envMapResolution = 2048;
			if (!(result = s_brdfLUTTexture.Initialize(envMapResolution, envMapResolution, ETT_FRAMEBUFFER_HDR_RG))) {
				printf("Fail to create brdfLUTTexture.\n");
				return result;
			}

			/*
			EnvironmentCaptureManager::AddCaptureProbes(cSphere(glm::vec3(-450, 10, 0), 600.f), 50.f, envMapResolution);
			EnvironmentCaptureManager::AddCaptureProbes(cSphere(glm::vec3(-225, 10, 0), 600.f), 50.f, envMapResolution);
			EnvironmentCaptureManager::AddCaptureProbes(cSphere(glm::vec3(-450, 290, 0), 600.f), 50.f, envMapResolution);
			EnvironmentCaptureManager::AddCaptureProbes(cSphere(glm::vec3(-225, 290, 0), 600.f), 50.f, envMapResolution);*/

			EnvironmentCaptureManager::AddCaptureProbes(cSphere(glm::vec3(0, 130, 0), 600.f), 50.f, envMapResolution);

			/*
			EnvironmentCaptureManager::AddCaptureProbes(cSphere(glm::vec3(225, 290, 0), 600.f), 50.f, envMapResolution);
			EnvironmentCaptureManager::AddCaptureProbes(cSphere(glm::vec3(450, 290, 0), 600.f), 50.f, envMapResolution);
			EnvironmentCaptureManager::AddCaptureProbes(cSphere(glm::vec3(225, 10, 0), 600.f), 50.f, envMapResolution);
			EnvironmentCaptureManager::AddCaptureProbes(cSphere(glm::vec3(450, 10, 0), 600.f), 50.f, envMapResolution);*/
			EnvironmentCaptureManager::BuildAccelerationStructure();

		}

		// Load models & textures
		{
			std::string _path = "Contents/models/arrow.model";
			if (!(result = Graphics::cModel::s_manager.Load(_path, s_arrowHandle)))
			{
				printf("Failed to Load arrow model!\n");
				return result;
			}

			_path = "Contents/models/cube.model";
			if (!(result = Graphics::cModel::s_manager.Load(_path, s_cubeHandle)))
			{
				printf("Failed to Load cube model!\n");
				return result;
			}

			_path = "Contents/models/quad.model";
			if (!(result = Graphics::cModel::s_manager.Load(_path, s_quadHandle)))
			{
				printf("Failed to Load quad model!\n");
				return result;
			}
			std::vector<float> _points;
			_points.push_back(0.0f); _points.push_back(0.0f); _points.push_back(0.0f);
			std::vector<GLuint> _indices;
			if (!(result == cMesh::s_manager.Load("Point", s_point, EMT_Point, _points, _indices)))
			{
				printf("Failed to Load point!\n");
				return result;
			}
		}
		// Load textures
		{
			std::string _path = "HDR/spruit_sunrise_2k.png";
			_path = Assets::ProcessPathTex(_path);
			if (!(result = cTexture::s_manager.Load(_path, s_spruitSunRise_HDR)))
			{
				printf("Failed to LoadspruitSunRise_HDR texture!\n");
				return result;
			}
			_path = "teapot_disp.png";
			_path = Assets::ProcessPathTex(_path);
			if (!(result = cTexture::s_manager.Load(_path, g_quadTeapotDisplacementMapHandle)))
			{
				printf("Failed to quadTeapot DisplacementMap texture!\n");
				return result;
			}
		}

		// Enable opengl features
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		GLint MaxPatchVertices = 0;
		glGetIntegerv(GL_MAX_PATCH_VERTICES, &MaxPatchVertices);
		printf("Max supported patch vertices %d\n", MaxPatchVertices);
		glPatchParameteri(GL_PATCH_VERTICES, 3);

		assert(GL_NO_ERROR == glGetError());

		return result;
	}

	void PreRenderFrame()
	{
		/** 1. Wait for data being submitted here */
		// Acquire the lock
		std::unique_lock<std::mutex> _mlock(s_graphicMutex);
		// Wait until the conditional variable is signaled
		s_whenAllDataHasBeenSubmittedFromApplicationThread.wait(_mlock);

		// After data has been submitted, swap the data
		std::swap(s_dataSubmittedByApplicationThread, s_dataRenderingByGraphicThread);
		// Notify the application thread that data is swapped and it is ready for receiving new data
		s_whenDataHasBeenSwappedInRenderThread.notify_one();

		s_uniformBuffer_ClipPlane.Update(&s_dataRenderingByGraphicThread->s_ClipPlane);

		/** 2. Convert all equirectangular HDR maps to cubemap */
		{
			s_currentEffect = Graphics::GetEffectByKey(EET_HDRToCubemap);
			s_currentEffect->UseEffect();

			s_currentEffect->SetInteger("rectangularHDRMap", 0);
			cTexture* _hdr = cTexture::s_manager.Get(s_spruitSunRise_HDR);
			_hdr->UseTexture(GL_TEXTURE0);

			glDisable(GL_CULL_FACE);
			s_cubemapProbe.StartCapture();
			for (size_t i = 0; i < 6; ++i)
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, s_cubemapProbe.GetCubemapTextureID(), 0);
				assert(GL_NO_ERROR == glGetError());
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				UniformBufferFormats::sFrame _cubemapFrameData(s_cubemapProbe.GetProjectionMat4(), s_cubemapProbe.GetViewMat4(i));
				_cubemapFrameData.ViewPosition = s_cubemapProbe.GetPosition();
				s_uniformBuffer_frame.Update(&_cubemapFrameData);

				// Render cube
				cModel* _cube = cModel::s_manager.Get(s_cubeHandle);
				if (_cube) { _cube->RenderWithoutMaterial(); }
			}

			s_cubemapProbe.StopCapture();
			glEnable(GL_CULL_FACE);
			s_currentEffect->UnUseEffect();
			// After capturing the scene, generate the mip map by opengl itself
			glBindTexture(GL_TEXTURE_CUBE_MAP, s_cubemapProbe.GetCubemapTextureID());
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		}
		/** 3. start generate BRDF LUTTexture */
		{
			s_currentEffect = Graphics::GetEffectByKey(EET_BrdfIntegration);
			s_currentEffect->UseEffect();
			s_brdfLUTTexture.Write();

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			cTransform quadTransform;
			quadTransform.SetRotation(glm::vec3(glm::radians(90.f), 0, 0));

			quadTransform.Update();
			s_uniformBuffer_drawcall.Update(&UniformBufferFormats::sDrawCall(quadTransform.M(), quadTransform.TranspostInverse()));

			// Render quad
			cModel* _quad = cModel::s_manager.Get(s_quadHandle);
			if (_quad) {
				_quad->RenderWithoutMaterial();
			}

			s_brdfLUTTexture.UnWrite();
			s_currentEffect->UnUseEffect();
		}

		/** 4. Start to render pass one by one */
		EnvironmentCaptureManager::CaptureEnvironment(s_dataRenderingByGraphicThread);

		printf("---------------------------------Pre-Rendering stage done.---------------------------------\n");
	}

	void RenderFrame()
	{
		/** 1. Wait for data being submitted here */
		// Acquire the lock
		std::unique_lock<std::mutex> _mlock(s_graphicMutex);
		// Wait until the conditional variable is signaled
		s_whenAllDataHasBeenSubmittedFromApplicationThread.wait(_mlock);

		// After data has been submitted, swap the data
		std::swap(s_dataSubmittedByApplicationThread, s_dataRenderingByGraphicThread);
		// Notify the application thread that data is swapped and it is ready for receiving new data
		s_whenDataHasBeenSwappedInRenderThread.notify_one();

		// Update cubemap weights before rendering, actually, this step should be done at the application thread
		EnvironmentCaptureManager::UpdatePointOfInterest(s_dataRenderingByGraphicThread->s_renderPasses[3].FrameData.ViewPosition);

		/** 2. Start to render pass one by one */
		for (size_t i = 0; i < s_dataRenderingByGraphicThread->s_renderPasses.size(); ++i)
		{
			// Update current render pass
			s_currentRenderPass = i;
			// Update frame data
			s_uniformBuffer_ClipPlane.Update(&s_dataRenderingByGraphicThread->s_ClipPlane);
			s_uniformBuffer_frame.Update(&s_dataRenderingByGraphicThread->s_renderPasses[i].FrameData);
			// Execute pass function
			s_dataRenderingByGraphicThread->s_renderPasses[i].RenderPassFunction();
		}

		//Gizmo_DrawDebugCaptureVolume();
	}

	void DirectionalShadowMap_Pass()
	{
		glDisable(GL_CULL_FACE);
		s_currentEffect = GetEffectByKey(EET_ShadowMap);
		s_currentEffect->UseEffect();
		cDirectionalLight* _directionalLight = &s_dataRenderingByGraphicThread->s_directionalLight;

		if (_directionalLight)
		{
			_directionalLight->SetupLight(s_currentEffect->GetProgramID(), 0);
			cFrameBuffer* _directionalLightFBO = _directionalLight->GetShadowMap();
			if (_directionalLightFBO && _directionalLight->IsShadowEnabled()) {

				// write buffer to the texture
				_directionalLightFBO->Write();

				glClearColor(0, 0, 0, 1.f);
				glClear(/*GL_COLOR_BUFFER_BIT | */GL_DEPTH_BUFFER_BIT);

				// Draw scenes
				RenderSceneWithoutMaterial();

				// switch back to original buffer
				_directionalLightFBO->UnWrite();
				assert(glGetError() == GL_NO_ERROR);
			}
		}
		s_currentEffect->UnUseEffect();
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}

	void PointLightShadowMap_Pass()
	{
		if (s_dataRenderingByGraphicThread->s_pointLights.size() <= 0) return;
		glDisable(GL_CULL_FACE);
		s_currentEffect = GetEffectByKey(EET_OmniShadowMap);
		s_currentEffect->UseEffect();

		s_currentEffect->SetInteger("displacementMap", 24);
		s_currentEffect->SetFloat("displaceIntensity", 20.0f);

		for (auto i = 0; i < s_dataRenderingByGraphicThread->s_pointLights.size(); ++i)
		{
			auto* it = &s_dataRenderingByGraphicThread->s_pointLights[i];
			cFrameBuffer* _pointLightFBO = it->GetShadowMap();
			if (_pointLightFBO) {

				// for each light, needs to update the frame data
				Graphics::UniformBufferFormats::sFrame _frameData_PointLightShadow;
				_frameData_PointLightShadow.ViewPosition = it->Transform()->Position();
				s_uniformBuffer_frame.Update(&_frameData_PointLightShadow);

				// point need extra uniform variables to be passed in to shader
				it->SetupLight(s_currentEffect->GetProgramID(), i);
				it->SetLightUniformTransform();
				// write buffer to the texture
				_pointLightFBO->Write();
				assert(glGetError() == GL_NO_ERROR);
				glClearColor(0, 0, 0, 1.f);
				glClear(GL_DEPTH_BUFFER_BIT);

				// Draw scenes
				RenderSceneWithoutMaterial();

				// switch back to original buffer
				_pointLightFBO->UnWrite();
				assert(glGetError() == GL_NO_ERROR);
			}
		}
		s_currentEffect->UnUseEffect();
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}

	void SpotLightShadowMap_Pass()
	{
		if (s_dataRenderingByGraphicThread->s_spotLights.size() <= 0) return;
		glDisable(GL_CULL_FACE);
		s_currentEffect = GetEffectByKey(EET_ShadowMap);
		s_currentEffect->UseEffect();

		for (auto i = 0; i < s_dataRenderingByGraphicThread->s_spotLights.size(); ++i)
		{
			auto* it = &s_dataRenderingByGraphicThread->s_spotLights[i];

			cFrameBuffer* _spotLightFB = it->GetShadowMap();
			if (_spotLightFB) {

				// for each light, needs to update the frame data
				Graphics::UniformBufferFormats::sFrame _frameData_SpotLightShadow(it->CalculateLightTransform());
				_frameData_SpotLightShadow.ViewPosition = it->Transform()->Position();
				s_uniformBuffer_frame.Update(&_frameData_SpotLightShadow);

				// write buffer to the texture
				_spotLightFB->Write();
				assert(glGetError() == GL_NO_ERROR);
				glClearColor(0, 0, 0, 1.f);
				glClear(GL_DEPTH_BUFFER_BIT);

				// Draw scenes
				RenderSceneWithoutMaterial();

				// switch back to original buffer
				_spotLightFB->UnWrite();
				assert(glGetError() == GL_NO_ERROR);
			}
		}
		s_currentEffect->UnUseEffect();
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}

	void SubmitClipPlaneData(const glm::vec4& i_plane0, const glm::vec4& i_plane1 /*= glm::vec4(0,0,0,0)*/, const glm::vec4& i_plane2 /*= glm::vec4(0, 0, 0, 0)*/, const glm::vec4& i_plane3 /*= glm::vec4(0, 0, 0, 0)*/)
	{
		s_dataSubmittedByApplicationThread->s_ClipPlane = UniformBufferFormats::sClipPlane(i_plane0, i_plane1, i_plane2, i_plane3);
	}

	void SubmitLightingData(const std::vector<cPointLight>& i_pointLights, const std::vector<cSpotLight>& i_spotLights, const cAmbientLight& i_ambientLight, const cDirectionalLight& i_directionalLight)
	{
		s_dataSubmittedByApplicationThread->s_pointLights = i_pointLights;
		s_dataSubmittedByApplicationThread->s_spotLights = i_spotLights;
		s_dataSubmittedByApplicationThread->s_directionalLight = i_directionalLight;
		s_dataSubmittedByApplicationThread->s_ambientLight = i_ambientLight;

	}

	void SubmitDataToBeRendered(const UniformBufferFormats::sFrame& i_frameData, const std::vector<std::pair<Graphics::cModel::HANDLE, cTransform>>& i_modelToTransform_map, void(*func_ptr)())
	{
		sPass inComingPasses;
		inComingPasses.FrameData = i_frameData;
		inComingPasses.ModelToTransform_map = i_modelToTransform_map;
		inComingPasses.RenderPassFunction = func_ptr;
		s_dataSubmittedByApplicationThread->s_renderPasses.push_back(inComingPasses);
	}

	void ClearApplicationThreadData()
	{
		s_dataSubmittedByApplicationThread->s_renderPasses.clear();
		s_dataSubmittedByApplicationThread->s_pointLights.clear();
		s_dataSubmittedByApplicationThread->s_spotLights.clear();
	}

	void SetCurrentPass(int i_currentPass)
	{
		s_currentRenderPass = i_currentPass;
	}

	void BlinnPhong_Pass()
	{

		// Bind effect
		{
			s_currentEffect = GetEffectByKey(EET_BlinnPhong);
			s_currentEffect->UseEffect();

		}
		// Clear color and buffers
		{
			// clear window
			glClearColor(s_clearColor.r, s_clearColor.g, s_clearColor.b, 0.f);
			// A lot of things can be cleaned like color buffer, depth buffer, so we need to specify what to clear
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		}

		// Update Lighting Data
		UpdateLightingData();

		// Start a draw call loop
		RenderScene();
		// clear program
		{
			s_currentEffect->UnUseEffect();
		}
		// Swap buffers happens in main rendering loop, not in this render function.
	}

	void UpdateInfoForPBR()
	{
		// Update Lighting Data
		UpdateLightingData();

		// update BRDF LUT texture
		{
			cTexture* _lutTexture = nullptr;
			if (&s_brdfLUTTexture && s_brdfLUTTexture.IsValid() && (_lutTexture = cTexture::s_manager.Get(s_brdfLUTTexture.GetTextureHandle())))
			{
				_lutTexture->UseTexture(GL_TEXTURE4);
			}
			else
				cTexture::UnBindTexture(GL_TEXTURE4, ETT_FRAMEBUFFER_HDR_RG);
		}

		const auto currentReadyCapturesCount = EnvironmentCaptureManager::GetReadyCapturesCount();
		const auto maxCubemapMixing = EnvironmentCaptureManager::MaximumCubemapMixingCount();
		const auto cubemapStartUnit = 5;
		for (size_t i = 0; i < currentReadyCapturesCount; ++i)
		{
			// Irradiance cube map
			{
				const cEnvProbe& _irradProbe = EnvironmentCaptureManager::GetCaptureProbesAt(i).IrradianceProbe;
				cTexture* _irrdianceMap = nullptr;
				if (_irradProbe.IsValid() && (_irrdianceMap = cTexture::s_manager.Get(_irradProbe.GetCubemapTextureHandle())))
				{
					_irrdianceMap->UseTexture(GL_TEXTURE0 + cubemapStartUnit + i);
				}
			}

			// pre-filter cube map
			{
				const cEnvProbe& _preFilteProbe = EnvironmentCaptureManager::GetCaptureProbesAt(i).PrefilterProbe;
				cTexture* _preFilterCubemap = nullptr;
				if (_preFilteProbe.IsValid() && (_preFilterCubemap = cTexture::s_manager.Get(_preFilteProbe.GetCubemapTextureHandle())))
				{
					_preFilterCubemap->UseTexture(GL_TEXTURE0 + cubemapStartUnit + maxCubemapMixing + i);
				}
			}
		}
		// Unbind last frame's textures
		for (int i = currentReadyCapturesCount; i < maxCubemapMixing; ++i)
		{
			cTexture::UnBindTexture(GL_TEXTURE0 + cubemapStartUnit + i, ETT_FRAMEBUFFER_HDR_CUBEMAP);
			cTexture::UnBindTexture(GL_TEXTURE0 + cubemapStartUnit + maxCubemapMixing + i, ETT_FRAMEBUFFER_HDR_MIPMAP_CUBEMAP);
		}
		assert(glGetError() == GL_NO_ERROR);

	}

	void PBR_Pass()
	{
		s_currentEffect = GetEffectByKey(EET_PBR_MR);
		s_currentEffect->UseEffect();

		glClearColor(s_clearColor.r, s_clearColor.g, s_clearColor.b, 0.f);
		// A lot of things can be cleaned like color buffer, depth buffer, so we need to specify what to clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		UpdateInfoForPBR();

		auto& renderList = s_dataRenderingByGraphicThread->s_renderPasses[s_currentRenderPass].ModelToTransform_map;
		const auto sphereOffset = renderList.size() - 25;
		// draw the space first
		for (int i = 0; i < sphereOffset; ++i)
		{
			// 1. Update draw call data
			s_uniformBuffer_drawcall.Update(&UniformBufferFormats::sDrawCall(renderList[i].second.M(), renderList[i].second.TranspostInverse()));
			// 2. Draw
			cModel* _model = cModel::s_manager.Get(renderList[i].first);
			if (_model) {
				_model->UpdateUniformVariables(s_currentEffect->GetProgramID());
				_model->Render();
			}
		}
		// assign different material for the sphere
		for (int i = 0; i < 5; ++i)
		{
			for (int j = 0; j < 5; ++j)
			{
				// 1. Update draw call data
				s_uniformBuffer_drawcall.Update(&UniformBufferFormats::sDrawCall(renderList[i * 5 + j + sphereOffset].second.M(), renderList[i * 5 + j + sphereOffset].second.TranspostInverse()));
				// 2. Draw
				cModel* _model = cModel::s_manager.Get(renderList[i * 5 + j + sphereOffset].first);
				if (_model) {
					Graphics::cMatPBRMR* _sphereMat = dynamic_cast<Graphics::cMatPBRMR*>(_model->GetMaterialAt());
					_sphereMat->UpdateMetalnessIntensity(1.f / 5.f * j + 0.1f);
					_sphereMat->UpdateRoughnessIntensity(0.9f - 1.f / 5.f *i);
					_model->UpdateUniformVariables(s_currentEffect->GetProgramID());
					_model->Render();
				}
			}
		}

		s_currentEffect->UnUseEffect();
	}

	void CubeMap_Pass()
	{
		// change depth function so depth test passes when values are equal to depth buffer's content
		glDisable(GL_CULL_FACE);

		s_currentEffect = GetEffectByKey(EET_Cubemap);
		s_currentEffect->UseEffect();

		for (auto it = s_dataRenderingByGraphicThread->s_renderPasses[s_currentRenderPass].ModelToTransform_map.begin();
			it != s_dataRenderingByGraphicThread->s_renderPasses[s_currentRenderPass].ModelToTransform_map.end(); ++it)
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
		glEnable(GL_CULL_FACE);
	}

	void Tessellation_Pass()
	{
		s_currentEffect = GetEffectByKey(EET_TessQuad);
		s_currentEffect->UseEffect();

		UpdateInfoForPBR();

		s_currentEffect->SetFloat("tessLevel", 100);
		s_currentEffect->SetInteger("displacementMap", 24);
		s_currentEffect->SetFloat("displaceIntensity", 20.0f);

		cTexture* _dispalcementMap = cTexture::s_manager.Get(g_quadTeapotDisplacementMapHandle);
		_dispalcementMap->UseTexture(GL_TEXTURE24);

		auto& renderList = s_dataRenderingByGraphicThread->s_renderPasses[s_currentRenderPass].ModelToTransform_map;
		auto& _pair = renderList[renderList.size() - 1];
		{
			// 1. Update draw call data
			s_uniformBuffer_drawcall.Update(&UniformBufferFormats::sDrawCall(_pair.second.M(), _pair.second.TranspostInverse()));
			// 2. Draw
			cModel* _model = cModel::s_manager.Get(_pair.first);
			if (_model) {
				_model->UpdateUniformVariables(s_currentEffect->GetProgramID());
				_model->Render(GL_PATCHES);
			}
		}

		s_currentEffect->UnUseEffect();
	}

	void Gizmo_RenderTransform()
	{
		s_currentEffect = GetEffectByKey(EET_Unlit);
		s_currentEffect->UseEffect();

		for (auto it = s_dataRenderingByGraphicThread->s_renderPasses[s_currentRenderPass].ModelToTransform_map.begin();
			it != s_dataRenderingByGraphicThread->s_renderPasses[s_currentRenderPass].ModelToTransform_map.end(); ++it)
		{
			// Get forward transform
			cTransform arrowTransform[3];
			{
				// Forward
				arrowTransform[0].SetRotation(it->second.Rotation() * glm::quat(glm::vec3(glm::radians(90.f), 0, 0)));
				// Right									  
				arrowTransform[1].SetRotation(it->second.Rotation() * glm::quat(glm::vec3(0, 0, glm::radians(90.f))));
				// Up											
				arrowTransform[2].SetRotation(it->second.Rotation() * glm::quat(glm::vec3(0, glm::radians(90.f), 0)));
			}

			cModel* _model = cModel::s_manager.Get(s_arrowHandle);
			cMatUnlit* _arrowMat = dynamic_cast<cMatUnlit*>(_model->GetMaterialAt());

			for (int i = 0; i < 3; ++i)
			{
				arrowTransform[i].SetPosition(it->second.Position());
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
	}

	void Gizmo_DrawDebugCaptureVolume() {

		s_currentEffect = GetEffectByKey(EET_DrawDebugCircles);
		s_currentEffect->UseEffect();
		auto _capturesRef = EnvironmentCaptureManager::GetCapturesReferences();
		for (int i = 0; i < _capturesRef.size(); ++i)
		{
			cMesh* _Point = cMesh::s_manager.Get(s_point);
			const cSphere& _outerBV = _capturesRef[i]->BV;
			const cSphere& _innerBV = _capturesRef[i]->InnerBV;
			cTransform _tempTransform;

			// outer
			s_currentEffect->SetFloat("radius", _outerBV.r());
			s_currentEffect->SetVec3("color", glm::vec3(1, 1, 1));
			_tempTransform.SetPosition(_outerBV.c());
			_tempTransform.Update();
			s_uniformBuffer_drawcall.Update(&UniformBufferFormats::sDrawCall(_tempTransform.M(), _tempTransform.TranspostInverse()));
			_Point->Render();

			// inner
			s_currentEffect->SetFloat("radius", _innerBV.r());
			s_currentEffect->SetVec3("color", glm::vec3(1, 0, 0));
			_tempTransform.SetPosition(_innerBV.c());
			_tempTransform.Update();
			s_uniformBuffer_drawcall.Update(&UniformBufferFormats::sDrawCall(_tempTransform.M(), _tempTransform.TranspostInverse()));
			_Point->Render();
		}
		s_currentEffect->UnUseEffect();
	}

	void Gizmo_RenderVertexNormal()
	{
		s_currentEffect = GetEffectByKey(EET_NormalDisplay);
		s_currentEffect->UseEffect();

		glClear(GL_DEPTH_BUFFER_BIT);

		RenderSceneWithoutMaterial();

		s_currentEffect->UnUseEffect();
	}
	void Gizmo_RenderTriangulation()
	{
		sWindowInput* _input = Application::GetCurrentApplication()->GetCurrentWindow()->GetWindowInput();
		if (!_input->IsKeyDown(GLFW_KEY_T)) return;

		s_currentEffect = GetEffectByKey(EET_TriangulationDisplay);
		s_currentEffect->UseEffect();

		glClear(GL_DEPTH_BUFFER_BIT);

		s_currentEffect->SetFloat("tessLevel", 100);
		s_currentEffect->SetInteger("displacementMap", 24);
		s_currentEffect->SetFloat("displaceIntensity", 20.0f);

		cTexture* _dispalcementMap = cTexture::s_manager.Get(g_quadTeapotDisplacementMapHandle);
		_dispalcementMap->UseTexture(GL_TEXTURE24);

		// loop through every single model
		for (auto it = s_dataRenderingByGraphicThread->s_renderPasses[s_currentRenderPass].ModelToTransform_map.begin();
			it != s_dataRenderingByGraphicThread->s_renderPasses[s_currentRenderPass].ModelToTransform_map.end(); ++it)
		{
			// 1. Update draw call data
			s_uniformBuffer_drawcall.Update(&UniformBufferFormats::sDrawCall(it->second.M(), it->second.TranspostInverse()));
			// 2. Draw
			cModel* _model = cModel::s_manager.Get(it->first);

			if (_model) {
				_model->UpdateUniformVariables(s_currentEffect->GetProgramID());
				cMatPBRMR* _pbrMat = dynamic_cast<cMatPBRMR*>(_model->GetMaterialAt());
				auto _handle = _pbrMat->GetNormalMapHandle();
				cTexture* _normal = cTexture::s_manager.Get(_handle);
				s_currentEffect->SetInteger("NormalMap", 3);
				_normal->UseTexture(GL_TEXTURE3);
				_model->RenderWithoutMaterial(GL_PATCHES);
				_normal->UnBindTexture(GL_TEXTURE3, ETT_FILE);
			}

		}

		_dispalcementMap->UnBindTexture(GL_TEXTURE24, ETT_FILE);
		s_currentEffect->UnUseEffect();
	}
	void RenderSceneWithoutMaterial()
	{
		// loop through every single model
		auto& renderMap = s_dataRenderingByGraphicThread->s_renderPasses[s_currentRenderPass].ModelToTransform_map;
		for (auto it = renderMap.begin(); it != renderMap.end(); ++it)
		{
			// 1. Update draw call data
			s_uniformBuffer_drawcall.Update(&UniformBufferFormats::sDrawCall(it->second.M(), it->second.TranspostInverse()));
			// 2. Draw
			cModel* _model = cModel::s_manager.Get(it->first);
			if (_model) { _model->RenderWithoutMaterial(); }
		}

	}

	void RenderScene(GLenum i_drawMode /*= GL_TRIANGLES*/)
	{
		// loop through every single model
		auto& renderMap = s_dataRenderingByGraphicThread->s_renderPasses[s_currentRenderPass].ModelToTransform_map;

		for (auto it = renderMap.begin(); it != renderMap.end(); ++it)
		{
			// 1. Update draw call data
			s_uniformBuffer_drawcall.Update(&UniformBufferFormats::sDrawCall(it->second.M(), it->second.TranspostInverse()));
			// 2. Draw
			cModel* _model = cModel::s_manager.Get(it->first);
			if (_model) {
				_model->UpdateUniformVariables(s_currentEffect->GetProgramID());
				_model->Render(i_drawMode);
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
		if (!(result = cMatPBRMR::GetUniformBuffer().CleanUp())) {
			printf("Fail to cleanup uniformBuffer_PBRMR\n");
		}
		if (!(result = s_uniformBuffer_Lighting.CleanUp())) {
			printf("Fail to cleanup uniformBuffer_Lighting\n");
		}
		if (!(result = s_uniformBuffer_ClipPlane.CleanUp())) {
			printf("Fail to cleanup uniformBuffer_ClipPlane\n");
		}
		cModel::s_manager.Release(s_arrowHandle);
		cModel::s_manager.Release(s_cubeHandle);
		cModel::s_manager.Release(s_quadHandle);
		cTexture::s_manager.Release(s_spruitSunRise_HDR);
		cMesh::s_manager.Release(s_point);
		cTexture::s_manager.Release(g_quadTeapotDisplacementMapHandle);
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
		if (s_ambientLight)
			s_ambientLight->CleanUpShadowMap();
		safe_delete(s_ambientLight);
		if (s_directionalLight)
			s_directionalLight->CleanUpShadowMap();
		safe_delete(s_directionalLight);

		s_cameraCapture.CleanUp();
		s_cubemapProbe.CleanUp();
		s_brdfLUTTexture.CleanUp();

		if (!(result = EnvironmentCaptureManager::CleanUp()))
		{
			printf("Fail to clean up Environment Capture Manager.\n");
		}
		return result;
	}

	//----------------------------------------------------------------------------------
	/** Getters */
	//----------------------------------------------------------------------------------

	cFrameBuffer* GetCameraCaptureFrameBuffer()
	{
		return &s_cameraCapture;
	}

	cEnvProbe* GetHDRtoCubemap()
	{
		return &s_cubemapProbe;
	}

	cFrameBuffer* GetBRDFLutFrameBuffer()
	{
		return &s_brdfLUTTexture;
	}

	Graphics::cUniformBuffer* GetUniformBuffer(const eUniformBufferType& i_uniformBufferType)
	{
		switch (i_uniformBufferType)
		{
		case UBT_Frame:
			return &s_uniformBuffer_frame;
			break;
		default:
			return nullptr;
			break;
		}

	}

	const Graphics::cModel::HANDLE& GetPrimitive(const EPrimitiveType& i_primitiveType)
	{
		switch (i_primitiveType)
		{
		case EPT_Cube:
			return s_cubeHandle;
			break;
		case EPT_Arrow:
			return s_arrowHandle;
			break;
		case EPT_Quad:
			return s_quadHandle;
			break;
		default:
			return cModel::HANDLE();
			break;
		}
	}

	//----------------------------------------------------------------------------------
	/** Effect related */
	//----------------------------------------------------------------------------------

	bool CreateEffect(const eEffectType& i_key, const char* i_vertexShaderPath, const char* i_fragmentShaderPath, const char* i_geometryShaderPath/* = ""*/, const char* const i_TCSPath /*= ""*/, const char* const i_TESPath/* = ""*/)
	{
		auto result = true;

		cEffect* newEffect = new  Graphics::cEffect();
		if (!(result = newEffect->CreateProgram(i_vertexShaderPath, i_fragmentShaderPath, i_geometryShaderPath, i_TCSPath, i_TESPath))) {
			printf("Fail to create default effect.\n");
			safe_delete(newEffect);
			return result;
		}

		s_KeyToEffect_map.insert({ i_key, newEffect });

		return result;
	}

	cEffect* GetEffectByKey(const eEffectType& i_key)
	{
		if (s_KeyToEffect_map.find(i_key) != s_KeyToEffect_map.end()) {
			return s_KeyToEffect_map.at(i_key);
		}
		return nullptr;
	}

	cEffect* GetCurrentEffect()
	{
		return s_currentEffect;
	}

	//----------------------------------------------------------------------------------
	/** Lighting related */
	//----------------------------------------------------------------------------------

	UniformBufferFormats::sLighting& GetGlobalLightingData()
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

	bool CreatePointLight(const glm::vec3& i_initialLocation, const Color& i_color, const GLfloat& i_radius, bool i_enableShadow, cPointLight*& o_pointLight)
	{
		auto result = true;
		if (result = (s_currentEffect->GetProgramID() == 0)) {
			printf("Can not create point light without a valid program id.\n");
			return result;
		}
		// TODO: lighting, range should be passed in
		cPointLight* newPointLight = new cPointLight(i_color, i_initialLocation, i_radius);
		newPointLight->SetupLight(s_currentEffect->GetProgramID(), s_pointLight_list.size());
		newPointLight->SetEnableShadow(i_enableShadow);
		newPointLight->CreateShadowMap(1024, 1024);
		o_pointLight = newPointLight;
		s_pointLight_list.push_back(newPointLight);
		return result;
	}

	bool CreateSpotLight(const glm::vec3& i_initialLocation, const glm::vec3& i_direction, const Color& i_color, const GLfloat& i_edge, const GLfloat& i_radius, bool i_enableShadow, cSpotLight*& o_spotLight)
	{
		auto result = true;
		if (result = (s_currentEffect->GetProgramID() == 0)) {
			printf("Can not create spot light without a valid program id.\n");
			return result;
		}
		cSpotLight* newSpotLight = new cSpotLight(i_color, i_initialLocation, glm::normalize(i_direction), i_edge, i_radius);
		newSpotLight->SetupLight(s_currentEffect->GetProgramID(), s_spotLight_list.size());
		newSpotLight->SetEnableShadow(i_enableShadow);
		newSpotLight->CreateShadowMap(1024, 1024);
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

	void UpdateLightingData()
	{
		s_dataRenderingByGraphicThread->s_ambientLight.Illuminate();

		for (int i = 0; i < s_dataRenderingByGraphicThread->s_spotLights.size(); ++i)
		{
			auto* it = &s_dataRenderingByGraphicThread->s_spotLights[i];

			it->SetupLight(s_currentEffect->GetProgramID(), i);
			it->Illuminate();
			it->SetLightUniformTransform();

			if (it->IsShadowEnabled()) {
				it->UseShadowMap(SHADOWMAP_START_TEXTURE_UNIT + i);
				it->GetShadowMap()->Read(GL_TEXTURE0 + SHADOWMAP_START_TEXTURE_UNIT + i);
			}
		}

		for (int i = 0; i < s_dataRenderingByGraphicThread->s_pointLights.size(); ++i)
		{
			auto* it = &s_dataRenderingByGraphicThread->s_pointLights[i];
			it->SetupLight(s_currentEffect->GetProgramID(), i);
			it->Illuminate();

			if (it->IsShadowEnabled()) {
				// has offset
				it->UseShadowMap(MAX_COUNT_PER_LIGHT + SHADOWMAP_START_TEXTURE_UNIT + i);
				it->GetShadowMap()->Read(GL_TEXTURE0 + MAX_COUNT_PER_LIGHT + SHADOWMAP_START_TEXTURE_UNIT + i);
			}
		}

		// Directional Light
		{
			cDirectionalLight* _directionalLight = &s_dataRenderingByGraphicThread->s_directionalLight;
			s_dataRenderingByGraphicThread->s_directionalLight.SetupLight(s_currentEffect->GetProgramID(), 0);
			_directionalLight->Illuminate();
			_directionalLight->SetLightUniformTransform();
			if (_directionalLight->IsShadowEnabled()) {
				constexpr GLuint _uniformID = SHADOWMAP_START_TEXTURE_UNIT + MAX_COUNT_PER_LIGHT * 2;
				_directionalLight->UseShadowMap(_uniformID);
				// GL_TEXTURE0 + POINT_LIGHT_COUNT + SPOT_LIGHT_COUNT + SHADOWMAP_OFFSET
				constexpr auto _textureID = GL_TEXTURE0 + SHADOWMAP_START_TEXTURE_UNIT + MAX_COUNT_PER_LIGHT * 2;
				_directionalLight->GetShadowMap()->Read(_textureID);
			}
		}

		s_globalLightingData.pointLightCount = s_dataRenderingByGraphicThread->s_pointLights.size();
		s_globalLightingData.spotLightCount = s_dataRenderingByGraphicThread->s_spotLights.size();
		s_uniformBuffer_Lighting.Update(&s_globalLightingData);

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
		s_whenDataHasBeenSwappedInRenderThread.wait_for(lck, std::chrono::milliseconds(timeToWait_inMilliseconds));
	}

}