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
#include "Cores/Utility/Profiler.h"
#include "Graphics/Texture/PboDownloader.h"
#include "Assignments/ParticleTest.h"
namespace Graphics {

	unsigned int g_currentRenderPass = 0;
	sDataRequiredToRenderAFrame g_dataRequiredToRenderAFrame[2];
	auto* g_dataSubmittedByApplicationThread = &g_dataRequiredToRenderAFrame[0];
	auto* g_dataRenderingByGraphicThread = &g_dataRequiredToRenderAFrame[1];

	sDataReturnToApplicationThread g_dataReturnToApplicationThread[2];
	auto * g_dataGetFromRenderThread = &g_dataReturnToApplicationThread[0];
	auto * g_dataUsedByApplicationThread = &g_dataReturnToApplicationThread[1];

	// Threading
	std::condition_variable g_whenAllDataHasBeenSubmittedFromApplicationThread;
	std::condition_variable g_whenDataHasBeenSwappedInRenderThread;
	std::condition_variable g_whenPreRenderFinish;
	std::condition_variable g_whenDataReturnToApplicationThreadHasSwapped;
	std::mutex g_graphicMutex;

	// Global data
	// ------------------------------------------------------------------------------------------------------------------------------------
	// Uniform buffer maps
	std::map<eUniformBufferType, cUniformBuffer> g_uniformBufferMap;

	// Pre-defined mesh & textures
	cModel g_cubeHandle;
	cModel g_arrowHandles[3];
	cModel g_quadHandle;
	cMesh::HANDLE s_point;
#ifdef ENABLE_CLOTH_SIM
	cMesh::HANDLE g_cloth;
	cMatPBRMR g_clothMat;
#endif // ENABLE_CLOTH_SIM
	cTexture::HANDLE s_spruitSunRise_HDR;
	cTexture::HANDLE g_ssaoNoiseTexture;
	cTexture::HANDLE g_pointLightIconTexture;
	cMaterial::HANDLE g_arrowMatHandles[2];
	PboDownloader g_pboDownloader;
	
	// Lighting data
	UniformBufferFormats::sLighting g_globalLightingData;
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
	const int g_noiseResolution = 4;
	// selection buffer
	cFrameBuffer g_selectionBuffer;
	// outline buffer
	cFrameBuffer g_outlineBuffer;

	// Effect
	// ------------------------------------------------------------------------------------------------------------------------------------
	std::map<eEffectType, cEffect*> g_KeyToEffect_map;
	cEffect* g_currentEffect;

	// Lighting
	// ------------------------------------------------------------------------------------------------------------------------------------
	// There are only one ambient and directional light
	cAmbientLight* g_ambientLight;
	cDirectionalLight* g_directionalLight;
	std::vector<cPointLight*> g_pointLight_list;
	std::vector<cSpotLight*> g_spotLight_list;

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
					"unlit/unlit_vert.glsl",
					"unlit/unlit_frag.glsl"))) {
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

				if (!(result = CreateEffect(EET_SSAO_Blur,
					"hdrShader/hdr_shader_vert.glsl",
					"ssao/ssao_blur_frag.glsl"
				))) {
					printf("Fail to create SSAO_Blur effect.\n");
					return result;
				}
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
			// Create cubemap displayer
			{
				if (!(result = CreateEffect(EET_SelectionBuffer,
					"selection/selection_vert.glsl",
					"selection/selection_frag.glsl"
				))) {
					printf("Fail to create selection buffer effect.\n");
					return result;
				}
			}
			// Create outline effect
			{
				if (!(result = CreateEffect(EET_Outline,
					"selection/outline/outline_vert.glsl",
					"unlit/unlit_frag.glsl"))) {
					printf("Fail to create outline effect.\n");
					return result;
				}

			}
			// draw billboard effect
			{
				if (!(result = CreateEffect(EET_Billboards,
					"billboard/billboard_vert.glsl",
					"billboard/billboard_frag.glsl"))) {
					printf("Fail to create billboard effect.\n");
					return result;
				}
			}
			// particle test
			{
				if (!(result = CreateEffect(EET_ParticleTest,
					"particle/particle_vert.glsl",
					"particle/particle_frag.glsl"))) {
					printf("Fail to create ParticleTest effect.\n");
					return result;
				}
			}
			// validate all programs
			for (auto it : g_KeyToEffect_map)
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

			if (!(result = g_selectionBuffer.Initialize(_window->GetBufferWidth(), _window->GetBufferHeight(), ETT_FRAMEBUFFER_RGB8)))
			{
				printf("Fail to create selection-Buffer.\n");
				return result;
			}
			if (!(result = g_outlineBuffer.Initialize(_window->GetBufferWidth(), _window->GetBufferHeight(), ETT_FRAMEBUFFER_STENCIL)))
			{
				printf("Fail to create outline-Buffer.\n");
				return result;
			}
			
			g_pboDownloader.init(GL_RGB, _window->GetBufferWidth(), _window->GetBufferHeight(), 2);

			//EnvironmentCaptureManager::AddCaptureProbes(cSphere(glm::vec3(-225, 230, 0), 600.f), 50.f, envMapResolution);

			EnvironmentCaptureManager::AddCaptureProbes(cSphere(glm::vec3(0, 230, 0), 600.f), 50.f, envMapResolution);

			//EnvironmentCaptureManager::AddCaptureProbes(cSphere(glm::vec3(225, 230, 0), 600.f), 50.f, envMapResolution);

			EnvironmentCaptureManager::BuildAccelerationStructure();


			// create profilers
			Profiler::CreateProfiler(Profiler::EPT_RenderAFrame);
			Profiler::CreateProfiler(Profiler::EPT_GBuffer);
			Profiler::CreateProfiler(Profiler::EPT_DeferredLighting);
			Profiler::CreateProfiler(Profiler::EPT_PointLightShadowMap);
			Profiler::CreateProfiler(Profiler::EPT_Selection);
		}



		// Load models & textures
		{
			g_cubeHandle = cModel("Contents/models/cube.model");

			g_quadHandle = cModel("Contents/models/quad.model");
			std::vector<float> _points;
			_points.push_back(0.0f); _points.push_back(0.0f); _points.push_back(0.0f);
			std::vector<GLuint> _indices;
			if (!(result == cMesh::s_manager.Load("Point", s_point, EMT_Point, _points, _indices)))
			{
				printf("Failed to Load point!\n");
				return result;
			}
			// Load arrow models
			g_arrowHandles[0] = cModel("Contents/models/arrow.model");
			g_arrowHandles[1] = cModel("Contents/models/arrow.model");
			g_arrowHandles[2] = cModel("Contents/models/arrow.model");
			cMaterial::s_manager.Duplicate(g_arrowHandles[0].GetMaterialAt(), g_arrowMatHandles[0]);
			cMaterial::s_manager.Duplicate(g_arrowHandles[0].GetMaterialAt(), g_arrowMatHandles[1]);
			g_arrowHandles[1].UpdateMaterial(g_arrowMatHandles[0]);
			g_arrowHandles[2].UpdateMaterial(g_arrowMatHandles[1]);
			GetEffectByKey(EET_Unlit)->UseEffect();
			for (int i = 0; i < 3; ++i)
			{
				g_arrowHandles[i].UpdateUniformVariables(GetEffectByKey(EET_Unlit)->GetProgramID());
				g_arrowHandles[i].IncreamentSelectableCount();
				cMatUnlit* _arrowMat = dynamic_cast<cMatUnlit*>(cMaterial::s_manager.Get(g_arrowHandles[i].GetMaterialAt()));
				_arrowMat->SetUnlitColor(Constants::g_arrowColor[i]);
			}
			GetEffectByKey(EET_Unlit)->UnUseEffect();

		}
		// Load textures
		{
			//std::string _path = "HDR/spruit_sunrise_2k.png";
			std::string _path = "HDR/HDR_ENV_Dynamic_01_s.hdr";
			_path = Assets::ProcessPathTex(_path);
			if (!(result = cTexture::s_manager.Load(_path, s_spruitSunRise_HDR)))
			{
				printf("Failed to Load spruitSunRise_HDR texture!\n");
				return result;
			}
			_path = "ssaoNoiseTexture";
			if (!(result = cTexture::s_manager.Load(_path, g_ssaoNoiseTexture, ETT_FRAMEBUFFER_RGB32, g_noiseResolution, g_noiseResolution)))
			{
				printf("Failed to load ssao_NoiseTexture!\n");
				return result;
			}
			_path = "PointLightIcon.png";
			_path = Assets::ProcessPathTex(_path);
			if (!(result = cTexture::s_manager.Load(_path, g_pointLightIconTexture, ETT_FILE_ALPHA)))
			{
				printf("Fail to load %s\n", _path.c_str());
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
				g_ssaoData.Samples[i] = glm::vec4(sample, 0.0);
			}
			const int noiseSampleCount = g_noiseResolution * g_noiseResolution;
			glm::vec3 ssaoNoise[noiseSampleCount];
			for (unsigned int i = 0; i < noiseSampleCount; i++)
			{
				glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
				ssaoNoise[i] = noise;
			}
			GLuint noiseTextureID = cTexture::s_manager.Get(g_ssaoNoiseTexture)->GetTextureID();
			glBindTexture(GL_TEXTURE_2D, noiseTextureID);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, g_noiseResolution, g_noiseResolution, 0, GL_RGB, GL_FLOAT, ssaoNoise);
			glBindTexture(GL_TEXTURE_2D, 0);

			g_ssaoData.power = 5;
			g_ssaoData.radius = 20.f;
		}

#ifdef ENABLE_CLOTH_SIM
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
			if (!(result == cMesh::s_manager.Load("Cloth", g_cloth, EMT_Mesh, _vertices, _indices)))
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
#endif // ENABLE_CLOTH_SIM

		ComputeShaderTest::Init();
		return result;
	}

	void PreRenderFrame()
	{
		// After data has been submitted, swap the data
		std::swap(g_dataSubmittedByApplicationThread, g_dataRenderingByGraphicThread);

		g_uniformBufferMap[UBT_ClipPlane].Update(&g_dataRenderingByGraphicThread->s_ClipPlane);

		/** 2. Convert all equirectangular HDR maps to cubemap */
		{
			g_currentEffect = Graphics::GetEffectByKey(EET_HDRToCubemap);
			g_currentEffect->UseEffect();
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
						g_cubeHandle.RenderWithoutMaterial();
					}
				}
			);

			glEnable(GL_CULL_FACE);
			g_currentEffect->UnUseEffect();
			// After capturing the scene, generate the mip map by opengl itself
			glBindTexture(GL_TEXTURE_CUBE_MAP, s_cubemapProbe.GetCubemapTextureID());
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		}
		//	printf("Finish generate equirectangular HDR maps to cubemap.\n");
			/** 3. start generate BRDF LUTTexture */
		{
			g_currentEffect = Graphics::GetEffectByKey(EET_BrdfIntegration);
			g_currentEffect->UseEffect();
			s_brdfLUTTexture.Write(
				[] {
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					RenderQuad();
					s_brdfLUTTexture.UnWrite();
				});

			g_currentEffect->UnUseEffect();
		}
		//printf("Finish generate BRDF LUT texture.\n");
		/** 4. Start to render pass one by one */
		EnvironmentCaptureManager::CaptureEnvironment(g_dataRenderingByGraphicThread);
		//printf("Finish capture environment.\n");
		g_whenPreRenderFinish.notify_one();
		printf("---------------------------------Pre-Rendering stage done.---------------------------------\n");
	}
	int renderCount = 0;
	void RenderFrame()
	{
		/** 1. Wait for data being submitted here */
		// Acquire the lock
		Profiler::StartRecording(6);
		std::unique_lock<std::mutex> _mlock(g_graphicMutex);
		/** 2.Wait until the conditional variable is signaled */
		g_whenAllDataHasBeenSubmittedFromApplicationThread.wait(_mlock);
		Profiler::StopRecording(6);

		// After data has been submitted, swap the data
		std::swap(g_dataSubmittedByApplicationThread, g_dataRenderingByGraphicThread);
		// Notify the application thread that data is swapped and it is ready for receiving new data
		g_whenDataHasBeenSwappedInRenderThread.notify_one();

		Profiler::StartRecording(Profiler::EPT_RenderAFrame);
		// For example calculate something here
		auto& _IO = g_dataSubmittedByApplicationThread->g_IO;
		// Update cubemap weights before rendering, actually, this step should be done at the application thread
		EnvironmentCaptureManager::UpdatePointOfInterest(g_dataRenderingByGraphicThread->g_renderPasses[3].FrameData.GetViewPosition());
		g_uniformBufferMap[UBT_ClipPlane].Update(&g_dataRenderingByGraphicThread->s_ClipPlane);
		g_uniformBufferMap[UBT_PostProcessing].Update(&g_dataRenderingByGraphicThread->s_PostProcessing);
		g_uniformBufferMap[UBT_SSAO].Update(&g_ssaoData);

#ifdef ENABLE_CLOTH_SIM
		cMesh::s_manager.Get(g_cloth)->UpdateBufferData(g_dataRenderingByGraphicThread->clothVertexData, ClothSim::VC * 14);
#endif // ENABLE_CLOTH_SIM
		/** 3. Start to render pass one by one */
		if (g_dataRenderingByGraphicThread->g_renderMode == ERM_ForwardShading)
		{
			ForwardShading();

		}
		else // if not forward shading, it is deferred shading
		{
			Profiler::StartRecording(2);
			DeferredShading();
			Profiler::StopRecording(2);
		}



		if (g_dataRenderingByGraphicThread->g_renderMode == ERM_DeferredShading || g_dataRenderingByGraphicThread->g_renderMode == ERM_ForwardShading)
		{
			Profiler::StartRecording(3);
			HDR_Pass();
			Profiler::StopRecording(3);

			// Clear the default buffer bit and copy hdr frame buffer's depth to the default frame buffer
			glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			glBlitNamedFramebuffer(g_hdrBuffer.fbo(), 0, 0, 0, g_hdrBuffer.GetWidth(), g_hdrBuffer.GetHeight(), 0, 0, g_hdrBuffer.GetWidth(), g_hdrBuffer.GetHeight(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);
			assert(GL_NO_ERROR == glGetError());

			// Render stuffs that needs depth info and needs not HDR info
			Profiler::StartRecording(4);
			RenderPointLightPosition();
			Profiler::StopRecording(4);
			//Gizmo_DrawDebugCaptureVolume();
			//RenderOmniShadowMap();


			Profiler::StartRecording(Profiler::EPT_Selection);
			Profiler::StartRecording(5);
			EditorPass();
			Profiler::StopRecording(5);
			Profiler::StopRecording(Profiler::EPT_Selection);

		}
		Profiler::StopRecording(Profiler::EPT_RenderAFrame);

		/** 4. After all render of this frame is done*/
		/*  
		// GPU profiling will take a long time because this need to wait until a GPU finish its rendering so this will take a long time, should avoid when unneccessary.
		 Profiler::GetProfilingTime(Profiler::EPT_RenderAFrame, g_dataGetFromRenderThread->g_deltaRenderAFrameTime);
		 Profiler::GetProfilingTime(Profiler::EPT_GBuffer, g_dataGetFromRenderThread->g_deltaGeometryTime);
		 Profiler::GetProfilingTime(Profiler::EPT_DeferredLighting, g_dataGetFromRenderThread->g_deltaDeferredLightingTime);
		 Profiler::GetProfilingTime(Profiler::EPT_PointLightShadowMap, g_dataGetFromRenderThread->g_deltaPointLightShadowMapTime);
		 Profiler::GetProfilingTime(Profiler::EPT_Selection, g_dataGetFromRenderThread->g_deltaSelectionTime);
		 */

		// swap data and notify data has been swapped.
		std::swap(g_dataGetFromRenderThread, g_dataUsedByApplicationThread);
		g_whenDataReturnToApplicationThreadHasSwapped.notify_one();
	}

	void SubmitClipPlaneData(const glm::vec4& i_plane0, const glm::vec4& i_plane1 /*= glm::vec4(0,0,0,0)*/, const glm::vec4& i_plane2 /*= glm::vec4(0, 0, 0, 0)*/, const glm::vec4& i_plane3 /*= glm::vec4(0, 0, 0, 0)*/)
	{
		g_dataSubmittedByApplicationThread->s_ClipPlane = UniformBufferFormats::sClipPlane(i_plane0, i_plane1, i_plane2, i_plane3);
	}

	void SubmitPostProcessingData(const UniformBufferFormats::sPostProcessing& i_ppData, float i_ssaoRadius, float i_ssaoPower)
	{
		g_dataSubmittedByApplicationThread->s_PostProcessing = i_ppData;

		g_ssaoData.radius = i_ssaoRadius;
		g_ssaoData.power = i_ssaoPower;
	}

	void SubmitLightingData(const std::vector<cPointLight>& i_pointLights, const std::vector<cSpotLight>& i_spotLights, const cAmbientLight& i_ambientLight, const cDirectionalLight& i_directionalLight)
	{
		g_dataSubmittedByApplicationThread->g_pointLights = i_pointLights;
		g_dataSubmittedByApplicationThread->g_spotLights = i_spotLights;
		g_dataSubmittedByApplicationThread->g_directionalLight = i_directionalLight;
		g_dataSubmittedByApplicationThread->g_ambientLight = i_ambientLight;

	}

	void SubmitIOData(const glm::vec2 & i_mousePos, const glm::vec2& i_mousePoseDelta, bool* i_buttonDowns)
	{
		sIO& _IO = g_dataSubmittedByApplicationThread->g_IO;
		_IO.MousePos = i_mousePos;
		_IO.dMousePos = i_mousePoseDelta;
		for (int i = 0; i < 3; ++i)
		{
			_IO.SetButton(i_buttonDowns[i], i);
		}
	}

	void SubmitSelectionData(uint32_t i_selectionID, const std::vector<std::pair<cModel, cTransform>>& i_modelTransformPairForSelection)
	{
		g_dataSubmittedByApplicationThread->g_selectingItemID = i_selectionID;
		g_dataSubmittedByApplicationThread->g_modelTransformPairForSelectionPass = i_modelTransformPairForSelection;

		auto& tempPair = g_dataSubmittedByApplicationThread->g_modelTransformPairForSelectionPass;
		const auto* frameData = &g_dataSubmittedByApplicationThread->g_renderPasses[3].FrameData;

		// render map + point light count + 3 transform gizmo
		tempPair.reserve(tempPair.size() + g_dataSubmittedByApplicationThread->g_pointLights.size() + 3);

		for (auto item : g_dataSubmittedByApplicationThread->g_pointLights)
		{
			g_quadHandle.SelectableID = item.SelectableID;
			cTransform pLightTransform(item.Transform.Position(), glm::quatLookAt(glm::normalize(frameData->GetViewPosition() - item.Transform.Position()), glm::vec3(frameData->ViewMatrix[0][0], frameData->ViewMatrix[0][1], frameData->ViewMatrix[0][2])), glm::vec3(10, 10, 10));
			tempPair.push_back({ g_quadHandle, pLightTransform });
		}
	}

#ifdef ENABLE_CLOTH_SIM
	void SubmitParticleData()
	{
		memcpy(g_dataSubmittedByApplicationThread->particles, ClothSim::g_positionData, sizeof(glm::vec3) * ClothSim::VC);
		memcpy(g_dataSubmittedByApplicationThread->clothVertexData, ClothSim::GetVertexData(), sizeof(float) * ClothSim::VC * 14);
}
#endif // ENABLE_CLOTH_SIM


	void SubmitGraphicSettings(const ERenderMode& i_renderMode)
	{
		g_dataSubmittedByApplicationThread->g_renderMode = i_renderMode;
	}

	void SubmitDataToBeRendered(const UniformBufferFormats::sFrame& i_frameData, const std::vector<std::pair<Graphics::cModel, cTransform>>& i_modelToTransform_map, void(*func_ptr)())
	{
		sPass inComingPasses;
		inComingPasses.FrameData = i_frameData;
		inComingPasses.ModelToTransform_map = i_modelToTransform_map;
		inComingPasses.RenderPassFunction = func_ptr;
		g_dataSubmittedByApplicationThread->g_renderPasses.push_back(inComingPasses);
	}

	void SetCurrentPass(int i_currentPass)
	{
		g_currentRenderPass = i_currentPass;
	}

	void RenderScene(cEffect* const i_effect, GLenum i_drawMode /*= GL_TRIANGLES*/)
	{
		// loop through every single model
		auto& renderMap = g_dataRenderingByGraphicThread->g_renderPasses[g_currentRenderPass].ModelToTransform_map;
		for (auto it = renderMap.begin(); it != renderMap.end(); ++it)
		{
			// 1. Update draw call data
			g_uniformBufferMap[UBT_Drawcall].Update(&UniformBufferFormats::sDrawCall(it->second.M(), it->second.TranspostInverse()));
			// 2. Draw
			if (i_effect) {
				it->first.UpdateUniformVariables(i_effect->GetProgramID());
				it->first.Render(i_drawMode);
			}
			else it->first.RenderWithoutMaterial(i_drawMode);
		}

	}

	bool CleanUp()
	{
		auto result = true;

		for (auto it : g_uniformBufferMap)
		{
			it.second.CleanUp();
		}

		for (int i = 0; i < 3; ++i)
		{
			g_arrowHandles[i].CleanUp();
		}
		g_cubeHandle.CleanUp();
		g_quadHandle.CleanUp();
		
		cMaterial::s_manager.Release(g_arrowMatHandles[0]);
		cMaterial::s_manager.Release(g_arrowMatHandles[1]);

		cTexture::s_manager.Release(s_spruitSunRise_HDR);
		cTexture::s_manager.Release(g_ssaoNoiseTexture);
		cTexture::s_manager.Release(g_pointLightIconTexture);
		g_pboDownloader.cleanUp();
		cMesh::s_manager.Release(s_point);
#ifdef ENABLE_CLOTH_SIM
		cMesh::s_manager.Release(g_cloth);
		g_clothMat.CleanUp();
#endif // ENABLE_CLOTH_SIM
		// Clean up effect
		for (auto it = g_KeyToEffect_map.begin(); it != g_KeyToEffect_map.end(); ++it)
			safe_delete(it->second);
		g_KeyToEffect_map.clear();

		// Clean up point light
		for (auto it = g_pointLight_list.begin(); it != g_pointLight_list.end(); ++it) {
			(*it)->CleanUpShadowMap();
			safe_delete(*it);
		}
		g_pointLight_list.clear();
		for (auto it = g_spotLight_list.begin(); it != g_spotLight_list.end(); ++it) {
			safe_delete(*it);
		}
		g_spotLight_list.clear();

		// Clean up ambient light
		if (g_ambientLight)
			g_ambientLight->CleanUpShadowMap();
		safe_delete(g_ambientLight);
		if (g_directionalLight)
			g_directionalLight->CleanUpShadowMap();
		safe_delete(g_directionalLight);
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
		g_selectionBuffer.CleanUp();
		g_outlineBuffer.CleanUp();
		if (!(result = EnvironmentCaptureManager::CleanUp()))
			printf("Fail to clean up Environment Capture Manager.\n");

		ComputeShaderTest::cleanUp();
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

	const Graphics::cModel& GetPrimitive(const EPrimitiveType& i_primitiveType)
	{
		switch (i_primitiveType)
		{
		case EPT_Cube:
			return g_cubeHandle;
			break;
		case EPT_Arrow:
			return g_arrowHandles[0];
			break;
		case EPT_Quad:
			return g_quadHandle;
			break;
		default:
			assert(false);
			return cModel();
			break;
		}
	}

	const Graphics::sDataReturnToApplicationThread& GetDataFromRenderThread()
	{
		std::unique_lock<std::mutex> lck(Application::GetCurrentApplication()->GetApplicationMutex());
		constexpr unsigned int timeToWait_inMilliseconds = 1;
		g_whenDataReturnToApplicationThreadHasSwapped.wait_for(lck, std::chrono::milliseconds(timeToWait_inMilliseconds));
		return *g_dataUsedByApplicationThread;
	}

	//----------------------------------------------------------------------------------
	/** Effect related */
	//----------------------------------------------------------------------------------

	bool CreateEffect(const eEffectType& i_key, const char* i_vertexShaderPath, const char* i_fragmentShaderPath, const char* i_geometryShaderPath/* = ""*/, const char* const i_TCSPath /*= ""*/, const char* const i_TESPath/* = ""*/)
	{
		auto result = true;

		cEffect* newEffect = new  Graphics::cEffect();
		if (!(result = newEffect->CreateProgram(i_key, i_vertexShaderPath, i_fragmentShaderPath, i_geometryShaderPath, i_TCSPath, i_TESPath))) {
			printf("Fail to create effect: %d.\n", i_key);
			safe_delete(newEffect);
			return result;
		}

		g_KeyToEffect_map.insert({ i_key, newEffect });

		return result;
	}

	cEffect* GetEffectByKey(const eEffectType& i_key)
	{
		//std::lock_guard<std::mutex> autoLock(g_graphicMutex);
		if (g_KeyToEffect_map.find(i_key) != g_KeyToEffect_map.end()) {
			return g_KeyToEffect_map.at(i_key);
		}
		return nullptr;
	}

	//----------------------------------------------------------------------------------
	/** Lighting related */
	//----------------------------------------------------------------------------------

	UniformBufferFormats::sLighting& GetGlobalLightingData()
	{
		return g_globalLightingData;
	}

	bool CreateAmbientLight(const Color& i_color, cAmbientLight*& o_ambientLight)
	{
		auto result = true;

		if (!(result = (g_ambientLight == nullptr))) {
			printf("Can not create duplicated ambient light.\n");
			return result;
		}
		g_ambientLight = new  cAmbientLight(i_color);
		g_ambientLight->SetupLight(0, 0);
		g_ambientLight->Intensity = 0.2f;
		o_ambientLight = g_ambientLight;

		return result;
	}

	bool CreatePointLight(const glm::vec3& i_initialLocation, const Color& i_color, const GLfloat& i_radius, bool i_enableShadow, cPointLight*& o_pointLight)
	{
		auto result = true;
		// TODO: lighting, range should be passed in
		cPointLight* newPointLight = new cPointLight(i_color, i_initialLocation, i_radius);
		newPointLight->SetEnableShadow(i_enableShadow);
		newPointLight->CreateShadowMap(2048, 2048);
		newPointLight->IncreamentSelectableCount();
		newPointLight->Intensity = 30.f;
		o_pointLight = newPointLight;
		g_pointLight_list.push_back(newPointLight);
		return result;
	}

	bool CreateSpotLight(const glm::vec3& i_initialLocation, const glm::vec3& i_direction, const Color& i_color, const GLfloat& i_edge, const GLfloat& i_radius, bool i_enableShadow, cSpotLight*& o_spotLight)
	{
		auto result = true;
		cSpotLight* newSpotLight = new cSpotLight(i_color, i_initialLocation, glm::normalize(i_direction), i_edge, i_radius);
		newSpotLight->SetEnableShadow(i_enableShadow);
		newSpotLight->CreateShadowMap(1024, 1024);
		o_spotLight = newSpotLight;
		g_spotLight_list.push_back(newSpotLight);

		return result;
	}

	bool CreateDirectionalLight(const Color& i_color, glm::vec3 i_direction, bool i_enableShadow, cDirectionalLight*& o_directionalLight)
	{
		auto result = true;
		cDirectionalLight* newDirectionalLight = new cDirectionalLight(i_color, glm::normalize(i_direction));
		newDirectionalLight->SetEnableShadow(i_enableShadow);
		newDirectionalLight->CreateShadowMap(2048, 2048);
		newDirectionalLight->Intensity = 3;
		o_directionalLight = newDirectionalLight;
		g_directionalLight = newDirectionalLight;
		return result;
	}

	//----------------------------------------------------------------------------------
	/** Threading related */
	//----------------------------------------------------------------------------------
	void Notify_DataHasBeenSubmited()
	{
		g_whenAllDataHasBeenSubmittedFromApplicationThread.notify_one();
	}

	void MakeApplicationThreadWaitForSwapingData(std::mutex& i_applicationMutex)
	{
		std::unique_lock<std::mutex> lck(g_graphicMutex);
		constexpr unsigned int timeToWait_inMilliseconds = 1;
		g_whenDataHasBeenSwappedInRenderThread.wait_for(lck, std::chrono::milliseconds(timeToWait_inMilliseconds));
	}

	void MakeApplicationThreadWaitUntilPreRenderFrameDone(std::mutex& i_applicationMutex)
	{
		std::unique_lock<std::mutex> lck(i_applicationMutex);
		g_whenPreRenderFinish.wait(lck);
	}

	void ClearApplicationThreadData()
	{
		g_dataSubmittedByApplicationThread->g_renderPasses.clear();
		g_dataSubmittedByApplicationThread->g_pointLights.clear();
		g_dataSubmittedByApplicationThread->g_spotLights.clear();
		g_dataSubmittedByApplicationThread->g_modelTransformPairForSelectionPass.clear();
	}
}