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
	// Lighting data
	UniformBufferFormats::sLighting s_globalLightingData;

	// Frame buffers
	// ------------------------------------------------------------------------------------------------------------------------------------
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


	// Functions
	// ------------------------------------------------------------------------------------------------------------------------------------

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
					"triangulationDisplayer/triangulation_vert.glsl",
					"normalDisplayer/normal_frag.glsl",
					"triangulationDisplayer/triangulation_geom.glsl"
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
		s_whenDataHasBeenSwappedInRenderThread.notify_all();

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
		s_whenDataHasBeenSwappedInRenderThread.notify_all();

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

	void SetCurrentPass(int i_currentPass)
	{
		s_currentRenderPass = i_currentPass;
	}

	void RenderScene(cEffect* const i_effect, GLenum i_drawMode /*= GL_TRIANGLES*/)
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
				if (i_effect) {
					_model->UpdateUniformVariables(s_currentEffect->GetProgramID());
					_model->Render(i_drawMode);
				}
				else _model->RenderWithoutMaterial(i_drawMode);
			}
		}

	}

	bool CleanUp()
	{
		auto result = true;
		if (!(result = s_uniformBuffer_frame.CleanUp()))
			printf("Fail to cleanup uniformBuffer_frame\n");
		if (!(result = s_uniformBuffer_drawcall.CleanUp()))
			printf("Fail to cleanup uniformBuffer_drawcall\n");
		if (!(result = cMatBlinn::GetUniformBuffer().CleanUp()))
			printf("Fail to cleanup uniformBuffer_MatBlinnPhong\n");
		if (!(result = cMatPBRMR::GetUniformBuffer().CleanUp()))
			printf("Fail to cleanup uniformBuffer_PBRMR\n");
		if (!(result = s_uniformBuffer_Lighting.CleanUp()))
			printf("Fail to cleanup uniformBuffer_Lighting\n");
		if (!(result = s_uniformBuffer_ClipPlane.CleanUp()))
			printf("Fail to cleanup uniformBuffer_ClipPlane\n");

		cModel::s_manager.Release(s_arrowHandle);
		cModel::s_manager.Release(s_cubeHandle);
		cModel::s_manager.Release(s_quadHandle);
		cTexture::s_manager.Release(s_spruitSunRise_HDR);
		cMesh::s_manager.Release(s_point);
		// Clean up effect
		for (auto it = s_KeyToEffect_map.begin(); it != s_KeyToEffect_map.end(); ++it)
			safe_delete(it->second);
		s_KeyToEffect_map.clear();

		// Clean up point light
		for (auto it = s_pointLight_list.begin(); it != s_pointLight_list.end(); ++it) {
			(*it)->CleanUpShadowMap();
			safe_delete(*it);
		}
		s_pointLight_list.clear();
		for (auto it = s_spotLight_list.begin(); it != s_spotLight_list.end(); ++it) {
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

		s_cubemapProbe.CleanUp();
		s_brdfLUTTexture.CleanUp();

		if (!(result = EnvironmentCaptureManager::CleanUp()))
			printf("Fail to clean up Environment Capture Manager.\n");

		return result;
	}

	//----------------------------------------------------------------------------------
	/** Getters */
	//----------------------------------------------------------------------------------

	cEnvProbe* GetHDRtoCubemap()
	{
		return &s_cubemapProbe;
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

		if (!(result = (s_ambientLight == nullptr))) {
			printf("Can not create duplicated ambient light.\n");
			return result;
		}
		s_ambientLight = new  cAmbientLight(i_color);
		s_ambientLight->SetupLight(0, 0);
		o_ambientLight = s_ambientLight;
		return result;
	}

	bool CreatePointLight(const glm::vec3& i_initialLocation, const Color& i_color, const GLfloat& i_radius, bool i_enableShadow, cPointLight*& o_pointLight)
	{
		auto result = true;
		// TODO: lighting, range should be passed in
		cPointLight* newPointLight = new cPointLight(i_color, i_initialLocation, i_radius);
		newPointLight->SetEnableShadow(i_enableShadow);
		newPointLight->CreateShadowMap(1024, 1024);
		o_pointLight = newPointLight;
		s_pointLight_list.push_back(newPointLight);
		return result;
	}

	bool CreateSpotLight(const glm::vec3& i_initialLocation, const glm::vec3& i_direction, const Color& i_color, const GLfloat& i_edge, const GLfloat& i_radius, bool i_enableShadow, cSpotLight*& o_spotLight)
	{
		auto result = true;
		cSpotLight* newSpotLight = new cSpotLight(i_color, i_initialLocation, glm::normalize(i_direction), i_edge, i_radius);
		newSpotLight->SetEnableShadow(i_enableShadow);
		newSpotLight->CreateShadowMap(1024, 1024);
		o_spotLight = newSpotLight;
		s_spotLight_list.push_back(newSpotLight);

		return result;
	}

	bool CreateDirectionalLight(const Color& i_color, glm::vec3 i_direction, bool i_enableShadow, cDirectionalLight*& o_directionalLight)
	{
		auto result = true;
		cDirectionalLight* newDirectionalLight = new cDirectionalLight(i_color, glm::normalize(i_direction));
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
		s_whenAllDataHasBeenSubmittedFromApplicationThread.notify_all();
	}

	void MakeApplicationThreadWaitForSwapingData(std::mutex& i_applicationMutex)
	{
		std::unique_lock<std::mutex> lck(i_applicationMutex);
		constexpr unsigned int timeToWait_inMilliseconds = 10;
		s_whenDataHasBeenSwappedInRenderThread.wait_for(lck, std::chrono::milliseconds(timeToWait_inMilliseconds));
	}

	void ClearApplicationThreadData()
	{
		s_dataSubmittedByApplicationThread->s_renderPasses.clear();
		s_dataSubmittedByApplicationThread->s_pointLights.clear();
		s_dataSubmittedByApplicationThread->s_spotLights.clear();
	}
}