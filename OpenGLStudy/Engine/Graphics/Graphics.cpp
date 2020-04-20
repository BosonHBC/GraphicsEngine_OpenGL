#include <map>
#include <random>
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
#include "Graphics/FrameBuffer/GeometryBuffer.h"

#include "Application/Window/WindowInput.h"

namespace Graphics {

	unsigned int s_currentRenderPass = 0;
	sDataRequiredToRenderAFrame s_dataRequiredToRenderAFrame[2];
	auto* s_dataSubmittedByApplicationThread = &s_dataRequiredToRenderAFrame[0];
	auto* s_dataRenderingByGraphicThread = &s_dataRequiredToRenderAFrame[1];

	// Threading
	std::condition_variable s_whenAllDataHasBeenSubmittedFromApplicationThread;
	std::condition_variable s_whenDataHasBeenSwappedInRenderThread;
	std::condition_variable s_whenPreRenderFinish;
	std::mutex s_graphicMutex;

	// Global data
	// ------------------------------------------------------------------------------------------------------------------------------------
	// Uniform buffer maps
	std::map<eUniformBufferType, cUniformBuffer> g_uniformBufferMap;

	// Pre-defined mesh & textures
	cModel::HANDLE s_cubeHandle;
	cModel::HANDLE s_arrowHandle;
	cModel::HANDLE s_quadHandle;
	cMesh::HANDLE s_point;
	cMesh::HANDLE g_cloth;
	cTexture::HANDLE s_spruitSunRise_HDR;
	cTexture::HANDLE g_ssaoNoiseTexture;
	cMatPBRMR g_clothMat;
	// Lighting data
	UniformBufferFormats::sLighting s_globalLightingData;
	UniformBufferFormats::sSSAO g_ssaoData;
	// Frame buffers
	// ------------------------------------------------------------------------------------------------------------------------------------
	cFrameBuffer g_omniShadowMaps[OMNI_SHADOW_MAP_COUNT];
	// Rectangular HDR map to cubemap
	cEnvProbe s_cubemapProbe;
	// the brdfLUTTexture for integrating the brdf
	cFrameBuffer s_brdfLUTTexture;
	// the frame buffer for capture HDR image
	cFrameBuffer g_hdrBuffer;
	// G-Buffer for deferred shading
	cGBuffer g_GBuffer;
	// SSAO frame buffer
	cFrameBuffer g_ssaoBuffer;
	// SSAO blur frame buffer
	cFrameBuffer g_ssao_blur_Buffer;
	const int noiseResolution = 4;
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
	bool CreateUniformBuffer(const eUniformBufferType& i_bufferFormat)
	{
		g_uniformBufferMap.insert({ i_bufferFormat, cUniformBuffer(i_bufferFormat) });
		if (g_uniformBufferMap[i_bufferFormat].Initialize(nullptr)) {
			g_uniformBufferMap[i_bufferFormat].Bind();
			return true;
		}
		else {
			printf("Fail to initialize uniformbuffer_%d\n", static_cast<int>(i_bufferFormat));
			return false;
		}
	}

	void Gizmo_DrawDebugCaptureVolume();
	void RenderQuad(const UniformBufferFormats::sFrame& i_frameData = UniformBufferFormats::sFrame(), const UniformBufferFormats::sDrawCall& i_drawCallData = UniformBufferFormats::sDrawCall());
	void RenderCube(const UniformBufferFormats::sFrame& i_frameData = UniformBufferFormats::sFrame(), const UniformBufferFormats::sDrawCall& i_drawCallData = UniformBufferFormats::sDrawCall());
	void RenderPointLightPosition();
	void RenderOmniShadowMap();

	void HDR_Pass();
	void GBuffer_Pass();
	void Deferred_Lighting_Pass();
	void DisplayGBuffer_Pass();
	float lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}
	void FixSamplerProblem(const eEffectType& i_effectKey)
	{
		// Fix sampler problem before validating the program

		cEffect* _effect = GetEffectByKey(i_effectKey);
		_effect->UseEffect();
		GLuint _programID = _effect->GetProgramID();
		char _charBuffer[64] = { '\0' };

		_effect->SetInteger("BrdfLUTMap", 4);
		_effect->SetInteger("AOMap", 5);

		const auto maxCubemapMixing = EnvironmentCaptureManager::MaximumCubemapMixingCount();
		constexpr auto cubemapStartID = IBL_CUBEMAP_START_TEXTURE_UNIT;
		for (size_t i = 0; i < maxCubemapMixing; ++i)
		{
			snprintf(_charBuffer, sizeof(_charBuffer), "IrradianceMap[%d]", i);
			_effect->SetInteger(_charBuffer, cubemapStartID + i);
			snprintf(_charBuffer, sizeof(_charBuffer), "PrefilterMap[%d]", i);
			_effect->SetInteger(_charBuffer, cubemapStartID + maxCubemapMixing + i);
		}
		for (int i = 0; i < OMNI_SHADOW_MAP_COUNT; ++i)
		{
			snprintf(_charBuffer, sizeof(_charBuffer), "spotlightShadowMap[%d]", i);
			_effect->SetInteger(_charBuffer, SHADOWMAP_START_TEXTURE_UNIT + i);
			snprintf(_charBuffer, sizeof(_charBuffer), "pointLightShadowMap[%d]", i);
			_effect->SetInteger(_charBuffer, SHADOWMAP_START_TEXTURE_UNIT + OMNI_SHADOW_MAP_COUNT + i);
		}
		_effect->SetInteger("directionalShadowMap", SHADOWMAP_START_TEXTURE_UNIT + OMNI_SHADOW_MAP_COUNT * 2);
		assert(GL_NO_ERROR == glGetError());
		_effect->UnUseEffect();
	}
	void ForwardShading();
	void DeferredShading();

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
			// Create OmniShadowmap effect
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
			// Create HDR effect
			{
				if (!(result = CreateEffect(EET_HDREffect,
					"hdrShader/hdr_shader_vert.glsl",
					"hdrShader/hdr_shader_frag.glsl"
				))) {
					printf("Fail to create HDREffect effect.\n");
					return result;
				}
				cEffect* hdrEffect = GetEffectByKey(EET_HDREffect);
				hdrEffect->UseEffect();
				hdrEffect->SetInteger("hdrBuffer", 0);
				hdrEffect->SetInteger("enableHDR", true);
				hdrEffect->UnUseEffect();
			}

			// Create G-Buffer
			{
				if (!(result = CreateEffect(EET_GBuffer,
					"deferredShading/GBuffer_vert.glsl",
					"deferredShading/GBuffer_frag.glsl"
				))) {
					printf("Fail to create GBuffer effect.\n");
					return result;
				}
			}
			// Create G-Buffer Display
			{
				if (!(result = CreateEffect(EET_GBufferDisplay,
					"hdrShader/hdr_shader_vert.glsl",
					"deferredShading/GBufferDisplay_frag.glsl"
				))) {
					printf("Fail to create GBuffer effect.\n");
					return result;
				}
				cEffect* bufferDisplayEffect = GetEffectByKey(EET_GBufferDisplay);
				bufferDisplayEffect->UseEffect();
				bufferDisplayEffect->SetInteger("gAlbedoMetallic", 0);
				bufferDisplayEffect->SetInteger("gNormalRoughness", 1);
				bufferDisplayEffect->SetInteger("gIOR", 2);
				bufferDisplayEffect->SetInteger("gDepth", 3);
				bufferDisplayEffect->SetInteger("gSSAOMap", 4);
				bufferDisplayEffect->UnUseEffect();
			}
			// Create Deferred-Lighting pass
			{
				if (!(result = CreateEffect(EET_DeferredLighting,
					"hdrShader/hdr_shader_vert.glsl",
					"deferredShading/DeferredLighting_frag.glsl"
				))) {
					printf("Fail to create Deferred lighting effect.\n");
					return result;
				}

				cEffect* dLighting = GetEffectByKey(EET_DeferredLighting);
				dLighting->UseEffect();
				dLighting->SetInteger("gAlbedoMetallic", 0);
				dLighting->SetInteger("gNormalRoughness", 1);
				dLighting->SetInteger("gIOR", 2);
				dLighting->SetInteger("gDepth", 3);
				dLighting->SetInteger("gSSAOMap", SHADOWMAP_START_TEXTURE_UNIT + OMNI_SHADOW_MAP_COUNT * 2 + 1);
				dLighting->UnUseEffect();
				FixSamplerProblem(EET_DeferredLighting);
			}

			// Create ssao and ssao_blur pass
			{
				if (!(result = CreateEffect(EET_SSAO,
					"hdrShader/hdr_shader_vert.glsl",
					"ssao/ssao_frag.glsl"
				))) {
					printf("Fail to create SSAO effect.\n");
					return result;
				}
				cEffect* ssaoEffect = GetEffectByKey(EET_SSAO);
				ssaoEffect->UseEffect();
				ssaoEffect->SetInteger("gNormalRoughness", 0);
				ssaoEffect->SetInteger("gDepth", 1);
				ssaoEffect->SetInteger("texNoise", 2);
				ssaoEffect->UnUseEffect();

				if (!(result = CreateEffect(EET_SSAO_Blur,
					"hdrShader/hdr_shader_vert.glsl",
					"ssao/ssao_blur_frag.glsl"
				))) {
					printf("Fail to create SSAO_Blur effect.\n");
					return result;
				}
				ssaoEffect = GetEffectByKey(EET_SSAO_Blur);
				ssaoEffect->UseEffect();
				ssaoEffect->SetInteger("ssaoInput", 0);
				ssaoEffect->UnUseEffect();
			}

			// Create cubemap displayer
			{
				if (!(result = CreateEffect(EET_CubemapDisplayer,
					"cubemap/cuebmapDisplay_vert.glsl",
					"cubemap/cubemapDisplay_frag.glsl"
				))) {
					printf("Fail to create Cubemap displayer effect.\n");
					return result;
				}
			}
			// validate all programs
			for (auto it : s_KeyToEffect_map)
			{
				it.second->ValidateProgram();
			}
		}
		result = CreateUniformBuffer(UBT_Frame);
		result = CreateUniformBuffer(UBT_Drawcall);
		result = CreateUniformBuffer(UBT_Lighting);
		result = CreateUniformBuffer(UBT_ClipPlane);
		result = CreateUniformBuffer(UBT_PostProcessing);
		result = CreateUniformBuffer(UBT_SSAO);
		assert(result);
		
		// Initialize omni shadow maps
		{
			for (int i = 0; i < OMNI_SHADOW_MAP_COUNT; ++i)
			{
				if (!(result = g_omniShadowMaps[i].Initialize(2048, 2048, ETT_FRAMEBUFFER_OMNI_SHADOWMAP))) {
					printf("Fail to create omni shadow map[%d].\n", i);
					return result;
				}
			}
		}
		// Initialize environment probes
		{
			// This is for changing rect hdr map to cubemap
			if (!(result = s_cubemapProbe.Initialize(10, 2048, 2048, ETT_FRAMEBUFFER_HDR_CUBEMAP))) {
				printf("Fail to create cubemap probe.\n");
				return result;
			}

			constexpr GLuint envMapResolution = 2048;
			if (!(result = s_brdfLUTTexture.Initialize(envMapResolution, envMapResolution, ETT_FRAMEBUFFER_RG16))) {
				printf("Fail to create brdfLUTTexture.\n");
				return result;
			}
			cWindow* _window = Application::GetCurrentApplication()->GetCurrentWindow();
			if (!(result = g_hdrBuffer.Initialize(_window->GetBufferWidth(), _window->GetBufferHeight(), ETT_FRAMEBUFFER_RGBA16)))
			{
				printf("Fail to create g_hdrBuffer.\n");
				return result;
			}
			if (!(result = g_GBuffer.Initialize(_window->GetBufferWidth(), _window->GetBufferHeight())))
			{
				printf("Fail to create G-Buffer.\n");
				return result;
			}
			if (!(result = g_ssaoBuffer.Initialize(_window->GetBufferWidth(), _window->GetBufferHeight(), ETT_FRAMEBUFFER_R16)))
			{
				printf("Fail to create SSAO-Buffer.\n");
				return result;
			}
			if (!(result = g_ssao_blur_Buffer.Initialize(_window->GetBufferWidth(), _window->GetBufferHeight(), ETT_FRAMEBUFFER_R16)))
			{
				printf("Fail to create SSAO-Blur-Buffer.\n");
				return result;
			}
			
			EnvironmentCaptureManager::AddCaptureProbes(cSphere(glm::vec3(-225, 230, 0), 600.f), 50.f, envMapResolution);

			EnvironmentCaptureManager::AddCaptureProbes(cSphere(glm::vec3(0, 230, 0), 600.f), 50.f, envMapResolution);

			EnvironmentCaptureManager::AddCaptureProbes(cSphere(glm::vec3(225, 230, 0), 600.f), 50.f, envMapResolution);
		
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
			_path = "ssaoNoiseTexture";
			if (!(result = cTexture::s_manager.Load(_path, g_ssaoNoiseTexture, ETT_FRAMEBUFFER_RGB32, noiseResolution, noiseResolution)))
			{
				printf("Failed to ssao_NoiseTexture!\n");
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
		printf("Max supported patch vertices: %d\n", MaxPatchVertices);
		glPatchParameteri(GL_PATCH_VERTICES, 3);

		GLint MaxTextureUnit = 0;
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &MaxTextureUnit);
		printf("Max supported textures per shader: %d\n", MaxTextureUnit);

		assert(GL_NO_ERROR == glGetError());

		// Generate ssao samples and noise texture
		{
			std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
			std::default_random_engine generator;

			for (unsigned int i = 0; i < SSAO_MAX_SAMPLECOUNT; ++i)
			{
				glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
				sample = glm::normalize(sample);
				sample *= randomFloats(generator);
				float scale = float(i) / SSAO_MAX_SAMPLECOUNT;

				// scale samples s.t. they're more aligned to center of kernel
				scale = lerp(0.1f, 1.0f, scale * scale);
				sample *= scale;
				g_ssaoData.Samples[i] = glm::vec4(sample,0.0);
			}
			const int noiseSampleCount = noiseResolution * noiseResolution;
			glm::vec3 ssaoNoise[noiseSampleCount];
			for (unsigned int i = 0; i < noiseSampleCount; i++)
			{
				glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
				ssaoNoise[i] = noise;
			}
			GLuint noiseTextureID = cTexture::s_manager.Get(g_ssaoNoiseTexture)->GetTextureID();
			glBindTexture(GL_TEXTURE_2D, noiseTextureID);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, noiseResolution, noiseResolution, 0, GL_RGB, GL_FLOAT, ssaoNoise);
			glBindTexture(GL_TEXTURE_2D, 0);

			g_ssaoData.power = 5;
			g_ssaoData.radius = 20.f;
		}

		// Initialize Cloth simulation progress
		{
			// Set up initial position for cloths
			float distBetweenVertex = CLOTH_LENGTH / (CLOTH_RESOLUTION - 1);
			for (size_t i = 0; i < CLOTH_RESOLUTION; i++)
			{
				for (int j = 0; j < CLOTH_RESOLUTION; ++j)
				{
					ClothSim::g_particles[i * CLOTH_RESOLUTION + j] = ClothSim::sParticle(glm::vec3(-100 + j * distBetweenVertex, 280.f - 0, i * distBetweenVertex - 200));
					if ((j + 1) % (CLOTH_RESOLUTION / 5) == 1 && i == 0)
						ClothSim::g_particles[i * CLOTH_RESOLUTION + j].isFixed = true;
				}
			}
			ClothSim::g_particles[0].isFixed = true;
			ClothSim::g_particles[CLOTH_RESOLUTION - 1].isFixed = true;
			ClothSim::InitializeNeghbors();

			auto _indices = ClothSim::GetIndexData();
			std::vector<float> _vertices;
			_vertices.resize(ClothSim::VC * 14);
			if (!(result == cMesh::s_manager.Load("Cloth", g_cloth, EMT_Mesh, _vertices,_indices)))
			{
				printf("Failed to Load Cloth!\n");
				return result;
			}
			if (!(result == g_clothMat.Initialize("Contents/materials/pbrFabrics.material")))
			{
				printf("Failed to Cloth material !\n");
				return result;
			}
		}
		return result;
	}

	void PreRenderFrame()
	{
		// After data has been submitted, swap the data
		std::swap(s_dataSubmittedByApplicationThread, s_dataRenderingByGraphicThread);

		g_uniformBufferMap[UBT_ClipPlane].Update(&s_dataRenderingByGraphicThread->s_ClipPlane);

		/** 2. Convert all equirectangular HDR maps to cubemap */
		{
			s_currentEffect = Graphics::GetEffectByKey(EET_HDRToCubemap);
			s_currentEffect->UseEffect();

			s_currentEffect->SetInteger("rectangularHDRMap", 0);
			cTexture* _hdr = cTexture::s_manager.Get(s_spruitSunRise_HDR);
			_hdr->UseTexture(GL_TEXTURE0);

			glDisable(GL_CULL_FACE);
			s_cubemapProbe.StartCapture(
				[] {
					for (size_t i = 0; i < 6; ++i)
					{
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, s_cubemapProbe.GetCubemapTextureID(), 0);
						assert(GL_NO_ERROR == glGetError());
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

						UniformBufferFormats::sFrame _cubemapFrameData(s_cubemapProbe.GetProjectionMat4(), s_cubemapProbe.GetViewMat4(i));
						g_uniformBufferMap[UBT_Frame].Update(&_cubemapFrameData);

						// Render cube
						cModel* _cube = cModel::s_manager.Get(s_cubeHandle);
						if (_cube) { _cube->RenderWithoutMaterial(); }
					}
				}
			);

			glEnable(GL_CULL_FACE);
			s_currentEffect->UnUseEffect();
			// After capturing the scene, generate the mip map by opengl itself
			glBindTexture(GL_TEXTURE_CUBE_MAP, s_cubemapProbe.GetCubemapTextureID());
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		}
		//	printf("Finish generate equirectangular HDR maps to cubemap.\n");
			/** 3. start generate BRDF LUTTexture */
		{
			s_currentEffect = Graphics::GetEffectByKey(EET_BrdfIntegration);
			s_currentEffect->UseEffect();
			s_brdfLUTTexture.Write(
				[]{
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					RenderQuad();
					s_brdfLUTTexture.UnWrite();
				});

			s_currentEffect->UnUseEffect();
		}
		//printf("Finish generate BRDF LUT texture.\n");
		/** 4. Start to render pass one by one */
		EnvironmentCaptureManager::CaptureEnvironment(s_dataRenderingByGraphicThread);
		//printf("Finish capture environment.\n");
		s_whenPreRenderFinish.notify_one();
		printf("---------------------------------Pre-Rendering stage done.---------------------------------\n");
	}
	int renderCount = 0;
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
		EnvironmentCaptureManager::UpdatePointOfInterest(s_dataRenderingByGraphicThread->s_renderPasses[3].FrameData.GetViewPosition());
		g_uniformBufferMap[UBT_ClipPlane].Update(&s_dataRenderingByGraphicThread->s_ClipPlane);
		g_uniformBufferMap[UBT_PostProcessing].Update(&s_dataRenderingByGraphicThread->s_PostProcessing);
		g_uniformBufferMap[UBT_SSAO].Update(&g_ssaoData);
		cMesh::s_manager.Get(g_cloth)->UpdateBufferData(s_dataRenderingByGraphicThread->clothVertexData, ClothSim::VC * 14);
		/** 2. Start to render pass one by one */
		if (s_dataRenderingByGraphicThread->g_renderMode == ERM_ForwardShading)
		{
			ForwardShading();
		}
		else // if not forward shading, it is deferred shading
		{
			DeferredShading();
		}
		//renderCount++;
		//printf("Render thread count: %d\n", renderCount);
		Gizmo_DrawDebugCaptureVolume();
	}

	void SubmitClipPlaneData(const glm::vec4& i_plane0, const glm::vec4& i_plane1 /*= glm::vec4(0,0,0,0)*/, const glm::vec4& i_plane2 /*= glm::vec4(0, 0, 0, 0)*/, const glm::vec4& i_plane3 /*= glm::vec4(0, 0, 0, 0)*/)
	{
		s_dataSubmittedByApplicationThread->s_ClipPlane = UniformBufferFormats::sClipPlane(i_plane0, i_plane1, i_plane2, i_plane3);
	}

	void SubmitPostProcessingData(const float i_exposure)
	{
		s_dataSubmittedByApplicationThread->s_PostProcessing.Exposure = i_exposure;
	}

	void SubmitLightingData(const std::vector<cPointLight>& i_pointLights, const std::vector<cSpotLight>& i_spotLights, const cAmbientLight& i_ambientLight, const cDirectionalLight& i_directionalLight)
	{
		s_dataSubmittedByApplicationThread->s_pointLights = i_pointLights;
		s_dataSubmittedByApplicationThread->s_spotLights = i_spotLights;
		s_dataSubmittedByApplicationThread->s_directionalLight = i_directionalLight;
		s_dataSubmittedByApplicationThread->s_ambientLight = i_ambientLight;

	}

	void SubmitParticleData()
	{
		memcpy(s_dataSubmittedByApplicationThread->particles, ClothSim::g_positionData, sizeof(glm::vec3) * ClothSim::VC);
		memcpy(s_dataSubmittedByApplicationThread->clothVertexData, ClothSim::GetVertexData(), sizeof(float) * ClothSim::VC * 14);
	}

	void SubmitGraphicSettings(const ERenderMode& i_renderMode)
	{
		s_dataSubmittedByApplicationThread->g_renderMode = i_renderMode;
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
			g_uniformBufferMap[UBT_Drawcall].Update(&UniformBufferFormats::sDrawCall(it->second.M(), it->second.TranspostInverse()));
			// 2. Draw
			cModel* _model = cModel::s_manager.Get(it->first);
			if (_model) {
				if (i_effect) {
					_model->UpdateUniformVariables(i_effect->GetProgramID());
					_model->Render(i_drawMode);
				}
				else _model->RenderWithoutMaterial(i_drawMode);
			}
		}

	}

	bool CleanUp()
	{
		auto result = true;

		for (auto it : g_uniformBufferMap)
		{
			it.second.CleanUp();
		}

		cModel::s_manager.Release(s_arrowHandle);
		cModel::s_manager.Release(s_cubeHandle);
		cModel::s_manager.Release(s_quadHandle);
		cTexture::s_manager.Release(s_spruitSunRise_HDR);
		cTexture::s_manager.Release(g_ssaoNoiseTexture);
		cMesh::s_manager.Release(s_point);
		cMesh::s_manager.Release(g_cloth);
		g_clothMat.CleanUp();
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
		for (int i = 0; i < OMNI_SHADOW_MAP_COUNT; ++i)
		{
			g_omniShadowMaps[i].CleanUp();
		}
		s_cubemapProbe.CleanUp();
		s_brdfLUTTexture.CleanUp();
		g_hdrBuffer.CleanUp();
		g_GBuffer.CleanUp();
		g_ssaoBuffer.CleanUp();
		g_ssao_blur_Buffer.CleanUp();
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
		return &g_uniformBufferMap[i_uniformBufferType];
	}

	Graphics::cFrameBuffer* GetOmniShadowMapAt(int i_idx)
	{
		assert(i_idx < OMNI_SHADOW_MAP_COUNT);
		return &g_omniShadowMaps[i_idx];
	}

	Graphics::cModel::HANDLE GetPrimitive(const EPrimitiveType& i_primitiveType)
	{
		cModel::HANDLE _invalidHandle;
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
			return _invalidHandle;
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
		//std::lock_guard<std::mutex> autoLock(s_graphicMutex);
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
		newPointLight->CreateShadowMap(2048, 2048);
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
		s_whenAllDataHasBeenSubmittedFromApplicationThread.notify_one();
	}

	void MakeApplicationThreadWaitForSwapingData(std::mutex& i_applicationMutex)
	{
		std::unique_lock<std::mutex> lck(s_graphicMutex);
		constexpr unsigned int timeToWait_inMilliseconds = 1;
		s_whenDataHasBeenSwappedInRenderThread.wait_for(lck, std::chrono::milliseconds(timeToWait_inMilliseconds));
	}

	void MakeApplicationThreadWaitUntilPreRenderFrameDone(std::mutex& i_applicationMutex)
	{
		std::unique_lock<std::mutex> lck(i_applicationMutex);
		s_whenPreRenderFinish.wait(lck);
	}

	void ClearApplicationThreadData()
	{
		s_dataSubmittedByApplicationThread->s_renderPasses.clear();
		s_dataSubmittedByApplicationThread->s_pointLights.clear();
		s_dataSubmittedByApplicationThread->s_spotLights.clear();
	}
}