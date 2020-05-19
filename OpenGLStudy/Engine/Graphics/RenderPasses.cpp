#include "Graphics/Graphics.h"
#include "Graphics/EnvironmentCaptureManager.h"
#include "Material/Unlit/MatUnlit.h"
#include "Material/PBR_MR/MatPBRMR.h"
#include "Application/Window/WindowInput.h"
#include "Application/Application.h"
#include "Application/Window/Window.h"
#include "Graphics/FrameBuffer/GeometryBuffer.h"
#include "Assignments/ClothSimulation/SimulationParams.h"
#include <algorithm>
#include "Application/Window/WindowInput.h"
#include "Application/imgui/imgui.h"
#include "Cores/Actor/Actor.h"
#include "Cores/Utility/GPUProfiler.h"

namespace Graphics
{
	extern unsigned int g_currentRenderPass;
	extern cEffect* g_currentEffect;
	extern sDataRequiredToRenderAFrame* g_dataRenderingByGraphicThread;
	extern 	sDataReturnToApplicationThread * g_dataGetFromRenderThread;
	extern 	std::map<eUniformBufferType, cUniformBuffer> g_uniformBufferMap;
	extern UniformBufferFormats::sLighting g_globalLightingData;
	extern cFrameBuffer g_omniShadowMaps[OMNI_SHADOW_MAP_COUNT];
	extern cFrameBuffer s_brdfLUTTexture;
	extern cFrameBuffer g_hdrBuffer;
	extern cGBuffer g_GBuffer;
	extern cFrameBuffer g_ssaoBuffer;
	extern cFrameBuffer g_ssao_blur_Buffer;
	extern 	cFrameBuffer g_selectionBuffer;
	extern cFrameBuffer g_outlineBuffer;
	extern cModel g_cubeHandle;
	extern cModel g_arrowHandles[3];
	extern cModel g_quadHandle;
	extern cMesh::HANDLE s_point;
#ifdef ENABLE_CLOTH_SIM
	extern cMesh::HANDLE g_cloth;
	extern cMatPBRMR g_clothMat;
#endif // ENABLE_CLOTH_SIM
	extern cTexture::HANDLE g_ssaoNoiseTexture;
	extern cTexture::HANDLE g_pointLightIconTexture;

	const std::map<uint8_t, const char*> g_renderModeNameMap =
	{
		std::pair<ERenderMode, const char*>(ERM_ForwardShading, "ForwardShading"),
		std::pair<ERenderMode, const char*>(ERM_DeferredShading, "DeferredShading"),
		std::pair<ERenderMode, const char*>(ERM_Deferred_Albede, "Albedo"),
		std::pair<ERenderMode, const char*>(ERM_Deferred_Metallic, "Metallic"),
		std::pair<ERenderMode, const char*>(ERM_Deferred_Roughness, "Roughness"),
		std::pair<ERenderMode, const char*>(ERM_Deferred_Normal, "Normal"),
		std::pair<ERenderMode, const char*>(ERM_Deferred_IOR, "IOR"),
		std::pair<ERenderMode, const char*>(ERM_Deferred_Depth, "Depth"),
		std::pair<ERenderMode, const char*>(ERM_Deferred_WorldPos, "WorldPosition"),
		std::pair<ERenderMode, const char*>(ERM_SSAO, "SSAO")
	};
	bool g_bRenderOmniShaodowMap = false;
	// clear color
	Color s_clearColor;
	extern Color g_outlineColor;
	extern Color g_arrowColor[3];
	extern unsigned int queryIDGeometry[2];
	void RenderQuad(const UniformBufferFormats::sFrame& i_frameData = UniformBufferFormats::sFrame(), const UniformBufferFormats::sDrawCall& i_drawCallData = UniformBufferFormats::sDrawCall())
	{
		g_uniformBufferMap[UBT_Frame].Update(&i_frameData);
		g_uniformBufferMap[UBT_Drawcall].Update(&i_drawCallData);


		g_quadHandle.RenderWithoutMaterial();
	}

	void RenderCube(const UniformBufferFormats::sFrame& i_frameData = UniformBufferFormats::sFrame(), const UniformBufferFormats::sDrawCall& i_drawCallData = UniformBufferFormats::sDrawCall())
	{
		g_uniformBufferMap[UBT_Frame].Update(&i_frameData);
		g_uniformBufferMap[UBT_Drawcall].Update(&i_drawCallData);

		g_cubeHandle.RenderWithoutMaterial();

	}

	void RenderPointLightPosition()
	{
		// Draw point light hint
		//-----------------------------------------------------------------------------------------------
		g_currentEffect = GetEffectByKey(EET_Billboards);
		g_currentEffect->UseEffect();
		cTransform pLightBillBoardTransform;
		g_uniformBufferMap[UBT_Frame].Update(&g_dataRenderingByGraphicThread->g_renderPasses[3].FrameData);
		for (size_t i = 0; i < g_dataRenderingByGraphicThread->g_pointLights.size(); ++i)
		{
			pLightBillBoardTransform.SetTransform(g_dataRenderingByGraphicThread->g_pointLights[i].Transform.Position(), glm::quat(1, 0, 0, 0), glm::vec3(10));
			g_uniformBufferMap[UBT_Drawcall].Update(&UniformBufferFormats::sDrawCall(pLightBillBoardTransform.M(), pLightBillBoardTransform.TranspostInverse()));

			cTexture* _texture = cTexture::s_manager.Get(g_pointLightIconTexture);
			_texture->UseTexture(GL_TEXTURE0);

			g_quadHandle.RenderWithoutMaterial();
		}

		cTexture::UnBindTexture(GL_TEXTURE0, ETT_FILE_ALPHA);
		g_currentEffect->UnUseEffect();
	}


	void RenderOmniShadowMap()
	{
		if (ImGui::IsKeyPressed(GLFW_KEY_RIGHT_CONTROL))
		{
			g_bRenderOmniShaodowMap = !g_bRenderOmniShaodowMap;
		}

		if (!g_bRenderOmniShaodowMap) return;
		for (int i = 0; i < OMNI_SHADOW_MAP_COUNT; ++i)
		{
			g_currentEffect = GetEffectByKey(EET_CubemapDisplayer);
			g_currentEffect->UseEffect();

			g_currentEffect->SetInteger("cubemapTex", 0);
			g_omniShadowMaps[i].Read(GL_TEXTURE0);

			cTransform tempTr(glm::vec3(-300, 200, 300 - 150 * i), glm::quat(1, 0, 0, 0), glm::vec3(5, 5, 5));
			RenderCube(g_dataRenderingByGraphicThread->g_renderPasses[3].FrameData, UniformBufferFormats::sDrawCall(tempTr.M(), tempTr.TranspostInverse()));

			g_currentEffect->UnUseEffect();
		}
	}


	void UpdateLightingData()
	{
		g_dataRenderingByGraphicThread->g_ambientLight.Illuminate();

		for (size_t i = 0; i < g_dataRenderingByGraphicThread->g_spotLights.size(); ++i)
		{
			auto* it = &g_dataRenderingByGraphicThread->g_spotLights[i];

			it->SetupLight(g_currentEffect->GetProgramID(), i);
			it->Illuminate();
			it->SetLightUniformTransform();

			if (it->IsShadowEnabled()) {
				it->UseShadowMap(SHADOWMAP_START_TEXTURE_UNIT + i);
				it->GetShadowMap()->Read(GL_TEXTURE0 + SHADOWMAP_START_TEXTURE_UNIT + i);
			}
		}

		for (size_t i = 0; i < g_dataRenderingByGraphicThread->g_pointLights.size(); ++i)
		{
			auto* it = &g_dataRenderingByGraphicThread->g_pointLights[i];
			it->Illuminate();

			g_omniShadowMaps[it->ShadowMapIdx()].Read(GL_TEXTURE0 + OMNI_SHADOW_MAP_COUNT + SHADOWMAP_START_TEXTURE_UNIT + it->ShadowMapIdx());
		}

		// Directional Light
		{
			cDirectionalLight* _directionalLight = &g_dataRenderingByGraphicThread->g_directionalLight;
			g_dataRenderingByGraphicThread->g_directionalLight.SetupLight(g_currentEffect->GetProgramID(), 0);
			_directionalLight->Illuminate();
			_directionalLight->SetLightUniformTransform();
			if (_directionalLight->IsShadowEnabled()) {
				constexpr GLuint _uniformID = SHADOWMAP_START_TEXTURE_UNIT + OMNI_SHADOW_MAP_COUNT * 2;
				_directionalLight->UseShadowMap(_uniformID);
				// GL_TEXTURE0 + POINT_LIGHT_COUNT + SPOT_LIGHT_COUNT + SHADOWMAP_OFFSET
				constexpr auto _textureID = GL_TEXTURE0 + SHADOWMAP_START_TEXTURE_UNIT + OMNI_SHADOW_MAP_COUNT * 2;
				_directionalLight->GetShadowMap()->Read(_textureID);
			}
		}

		g_globalLightingData.pointLightCount = g_dataRenderingByGraphicThread->g_pointLights.size();
		g_globalLightingData.spotLightCount = g_dataRenderingByGraphicThread->g_spotLights.size();
		g_uniformBufferMap[UBT_Lighting].Update(&g_globalLightingData);

	}

	void DirectionalShadowMap_Pass()
	{
		cDirectionalLight* _directionalLight = &g_dataRenderingByGraphicThread->g_directionalLight;

		if (!_directionalLight || !_directionalLight->IsShadowEnabled()) return;

		glDisable(GL_CULL_FACE);
		g_currentEffect = GetEffectByKey(EET_ShadowMap);
		g_currentEffect->UseEffect();

		_directionalLight->SetupLight(g_currentEffect->GetProgramID(), 0);
		cFrameBuffer* _directionalLightFBO = _directionalLight->GetShadowMap();
		if (_directionalLightFBO && _directionalLight->IsShadowEnabled()) {

			// write buffer to the texture
			_directionalLightFBO->Write(
				[] {
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					// Draw scenes
					RenderScene(nullptr);
#ifdef ENABLE_CLOTH_SIM
					cMesh* _cloth = cMesh::s_manager.Get(g_cloth);
					cTransform defaultTransform;
					g_uniformBufferMap[UBT_Drawcall].Update(&UniformBufferFormats::sDrawCall(defaultTransform.M(), defaultTransform.TranspostInverse()));
					_cloth->Render();
#endif // ENABLE_CLOTH_SIM
				}
			);
			assert(glGetError() == GL_NO_ERROR);
		}

		g_currentEffect->UnUseEffect();
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}
	const int shadowMapResolution = 2048;
	const sRect g_subRectRefs[16] =
	{
		// 0-1. 1024 * 1024 * 2
		sRect(glm::vec2(0,0),																								shadowMapResolution / 2, shadowMapResolution / 2),
		sRect(glm::vec2(shadowMapResolution / 2,0),														shadowMapResolution / 2, shadowMapResolution / 2),
		// 2-7. 512 * 512 * 6
		sRect(glm::vec2(shadowMapResolution * 0 / 4, shadowMapResolution * 2 / 4),	shadowMapResolution / 4, shadowMapResolution / 4),
		sRect(glm::vec2(shadowMapResolution * 1 / 4, shadowMapResolution * 2 / 4),	shadowMapResolution / 4, shadowMapResolution / 4),
		sRect(glm::vec2(shadowMapResolution * 0 / 4, shadowMapResolution * 3 / 4),	shadowMapResolution / 4, shadowMapResolution / 4),
		sRect(glm::vec2(shadowMapResolution * 1 / 4, shadowMapResolution * 3 / 4),	shadowMapResolution / 4, shadowMapResolution / 4),
		sRect(glm::vec2(shadowMapResolution * 2 / 4, shadowMapResolution * 2 / 4),	shadowMapResolution / 4, shadowMapResolution / 4),
		sRect(glm::vec2(shadowMapResolution * 3 / 4, shadowMapResolution * 2 / 4),	shadowMapResolution / 4, shadowMapResolution / 4),
		// 8-15. 256 * 256 * 8
		sRect(glm::vec2(shadowMapResolution * 4 / 8, shadowMapResolution * 6 / 8),	shadowMapResolution / 8, shadowMapResolution / 8),
		sRect(glm::vec2(shadowMapResolution * 5 / 8, shadowMapResolution * 6 / 8),	shadowMapResolution / 8, shadowMapResolution / 8),
		sRect(glm::vec2(shadowMapResolution * 4 / 8, shadowMapResolution * 7 / 8),	shadowMapResolution / 8, shadowMapResolution / 8),
		sRect(glm::vec2(shadowMapResolution * 5 / 8, shadowMapResolution * 7 / 8),	shadowMapResolution / 8, shadowMapResolution / 8),
		sRect(glm::vec2(shadowMapResolution * 6 / 8, shadowMapResolution * 6 / 8),	shadowMapResolution / 8, shadowMapResolution / 8),
		sRect(glm::vec2(shadowMapResolution * 7 / 8, shadowMapResolution * 6 / 8),	shadowMapResolution / 8, shadowMapResolution / 8),
		sRect(glm::vec2(shadowMapResolution * 6 / 8, shadowMapResolution * 7 / 8),	shadowMapResolution / 8, shadowMapResolution / 8),
		sRect(glm::vec2(shadowMapResolution * 7 / 8, shadowMapResolution * 7 / 8),	shadowMapResolution / 8, shadowMapResolution / 8),
	};

	bool RetriveShadowMapIndexAndSubRect(int i_lightIdx, int& io_shadowmapIdx, int& io_resolutionIdx)
	{
		const int  N = g_omniShadowMaps[0].GetWidth(); // shadow map resolution
		if (i_lightIdx >= 0 && i_lightIdx < 10)	// 1K
		{
			io_resolutionIdx = (i_lightIdx % 2 == 0) ? 0 : 1;
			io_shadowmapIdx = i_lightIdx / 2;
		}
		if (i_lightIdx >= 10 && i_lightIdx < 40) // 512 * 512
		{
			io_resolutionIdx = (i_lightIdx - 10) % 6 + 2;
			io_shadowmapIdx = (i_lightIdx - 10) / 6;
		}
		if (i_lightIdx >= 40 && i_lightIdx < 80) // 256 * 256
		{
			io_resolutionIdx = (i_lightIdx - 40) % 8 + 8;
			io_shadowmapIdx = (i_lightIdx - 40) / 8;
		}
		return (io_shadowmapIdx < OMNI_SHADOW_MAP_COUNT && io_shadowmapIdx >= 0 && io_resolutionIdx >= 0 && io_resolutionIdx < 16);
	}

	void PointLightShadowMap_Pass()
	{
		if (g_dataRenderingByGraphicThread->g_pointLights.size() <= 0) return;

		std::sort(g_dataRenderingByGraphicThread->g_pointLights.begin(), g_dataRenderingByGraphicThread->g_pointLights.end(), [](cPointLight& const l1, cPointLight&  const l2) {
			return l1.Importance() > l2.Importance(); });
		// Now the point light list is sorted depends on their importance
		for (size_t i = 0; i < g_dataRenderingByGraphicThread->g_pointLights.size(); ++i)
		{
			auto* it = &g_dataRenderingByGraphicThread->g_pointLights[i];
			int shadowMapIdx = -1; int resolutionIdx = -1;
			if (RetriveShadowMapIndexAndSubRect(i, shadowMapIdx, resolutionIdx))
			{
				it->SetShadowmapIdxAndResolutionIdx(shadowMapIdx, resolutionIdx);
				it->ImportanceOrder = i;
			}
			else
				assert(false);
		}
		std::sort(g_dataRenderingByGraphicThread->g_pointLights.begin(), g_dataRenderingByGraphicThread->g_pointLights.end(), [](cPointLight& const l1, cPointLight&  const l2) {
			return l1.ShadowMapIdx() < l2.ShadowMapIdx(); });

		Profiler::StartRecording(Profiler::EPT_PointLightShadowMap);
		glDisable(GL_CULL_FACE);
		g_currentEffect = GetEffectByKey(EET_OmniShadowMap);
		g_currentEffect->UseEffect();
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_SCISSOR_TEST);
		// Now the point light list is sorted depends on their shadow map index
		for (size_t i = 0; i < g_dataRenderingByGraphicThread->g_pointLights.size(); ++i)
		{
			auto* it = &g_dataRenderingByGraphicThread->g_pointLights[i];

			// for each light, needs to update the frame data
			Graphics::UniformBufferFormats::sFrame _frameData_PointLightShadow(it->GetProjectionmatrix(), it->GetViewMatrix());
			g_uniformBufferMap[UBT_Frame].Update(&_frameData_PointLightShadow);

			// point need extra uniform variables to be passed in to shader
			it->SetupLight(g_currentEffect->GetProgramID(), i);
			it->SetLightUniformTransform();

			// write buffer to the texture
			sRect subRect = g_subRectRefs[it->ResolutionIdx()];

			g_omniShadowMaps[it->ShadowMapIdx()].WriteSubArea(
				[&] {
					glScissor(subRect.Min.x, subRect.Min.y, subRect.w(), subRect.h()); // set this up with the same inputs as the glViewport function
					glClear(GL_DEPTH_BUFFER_BIT);

					// loop through every single model
					auto& renderMap = g_dataRenderingByGraphicThread->g_renderPasses[g_currentRenderPass].ModelToTransform_map;
					for (auto it2 = renderMap.begin(); it2 != renderMap.end(); ++it2)
					{
						// 1. Transform the sphere to the mesh spaced coordinate, check if they have overlaps
						glm::mat4 _sphereToMeshMatrix = it2->second.MInv() * it->Transform.M();
						glm::vec3 tramsformedPosition = _sphereToMeshMatrix * glm::vec4(0, 0, 0, 1);
						float transformedRadius(_sphereToMeshMatrix[0][0]);
						// if there is no overlap, don't need to draw
						if (!it2->first.IntersectWithSphere(cSphere(tramsformedPosition, transformedRadius))) continue;

						// 2. Update draw call data
						g_uniformBufferMap[UBT_Drawcall].Update(&UniformBufferFormats::sDrawCall(it2->second.M(), it2->second.TranspostInverse()));
						// 3. Draw
						it2->first.RenderWithoutMaterial(GL_TRIANGLES);

					}
				}
			, subRect);
			assert(glGetError() == GL_NO_ERROR);

		}
		glDisable(GL_SCISSOR_TEST);
		g_currentEffect->UnUseEffect();
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		Profiler::StopRecording(Profiler::EPT_PointLightShadowMap);

	}

	void SpotLightShadowMap_Pass()
	{
		if (g_dataRenderingByGraphicThread->g_spotLights.size() <= 0) return;
		glDisable(GL_CULL_FACE);
		g_currentEffect = GetEffectByKey(EET_ShadowMap);
		g_currentEffect->UseEffect();

		for (size_t i = 0; i < g_dataRenderingByGraphicThread->g_spotLights.size(); ++i)
		{
			auto* it = &g_dataRenderingByGraphicThread->g_spotLights[i];

			cFrameBuffer* _spotLightFB = it->GetShadowMap();
			if (_spotLightFB) {

				// for each light, needs to update the frame data
				Graphics::UniformBufferFormats::sFrame _frameData_SpotLightShadow(it->GetProjectionmatrix(), it->GetViewMatrix());

				g_uniformBufferMap[UBT_Frame].Update(&_frameData_SpotLightShadow);

				// write buffer to the texture
				_spotLightFB->Write(
					[] {
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
						// Draw scenes
						RenderScene(nullptr);
					}
				);
				assert(glGetError() == GL_NO_ERROR);
			}
		}
		g_currentEffect->UnUseEffect();
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}

	void BlinnPhong_Pass()
	{
		g_currentEffect = GetEffectByKey(EET_BlinnPhong);
		g_currentEffect->UseEffect();
		glClearColor(s_clearColor.r, s_clearColor.g, s_clearColor.b, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Update Lighting Data
		UpdateLightingData();
		// Start a draw call loop
		RenderScene(g_currentEffect);
		g_currentEffect->UnUseEffect();
	}

	// Update lighting data, update BRDF-LUT texture, update environment captures for irradiance and pre-filtering mapping
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
				cTexture::UnBindTexture(GL_TEXTURE4, ETT_FRAMEBUFFER_RG16);
		}

		const auto currentReadyCapturesCount = EnvironmentCaptureManager::GetReadyCapturesCount();
		const auto maxCubemapMixing = EnvironmentCaptureManager::MaximumCubemapMixingCount();
		const int cubemapStartUnit = IBL_CUBEMAP_START_TEXTURE_UNIT;
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
		for (size_t i = currentReadyCapturesCount; i < maxCubemapMixing; ++i)
		{
			cTexture::UnBindTexture(GL_TEXTURE0 + cubemapStartUnit + i, ETT_FRAMEBUFFER_HDR_CUBEMAP);
			cTexture::UnBindTexture(GL_TEXTURE0 + cubemapStartUnit + maxCubemapMixing + i, ETT_FRAMEBUFFER_HDR_MIPMAP_CUBEMAP);
		}
		assert(glGetError() == GL_NO_ERROR);

	}

	void PBR_Pass()
	{
		g_currentEffect = GetEffectByKey(EET_PBR_MR);
		g_currentEffect->UseEffect();

		glClearColor(s_clearColor.r, s_clearColor.g, s_clearColor.b, 0.f);
		// A lot of things can be cleaned like color buffer, depth buffer, so we need to specify what to clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		UpdateInfoForPBR();

		RenderScene(g_currentEffect);

#ifdef ENABLE_CLOTH_SIM
		cMesh* _cloth = cMesh::s_manager.Get(g_cloth);
		cTransform defaultTransform;
		g_uniformBufferMap[UBT_Drawcall].Update(&UniformBufferFormats::sDrawCall(defaultTransform.M(), defaultTransform.TranspostInverse()));

		g_clothMat.UpdateUniformVariables(g_currentEffect->GetProgramID());
		g_clothMat.UseMaterial();
		glDisable(GL_CULL_FACE);
		_cloth->Render();
		glEnable(GL_CULL_FACE);
		g_clothMat.CleanUpMaterialBind();
#endif // ENABLE_CLOTH_SIM

		g_currentEffect->UnUseEffect();
	}

	void CubeMap_Pass()
	{
		// change depth function so depth test passes when values are equal to depth buffer's content
		glDisable(GL_CULL_FACE);

		g_currentEffect = GetEffectByKey(EET_Cubemap);
		g_currentEffect->UseEffect();
		RenderScene(g_currentEffect);
		g_currentEffect->UnUseEffect();

		// set depth function back to default
		glEnable(GL_CULL_FACE);
	}

	void HDR_Pass()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		g_currentEffect = GetEffectByKey(EET_HDREffect);
		g_currentEffect->UseEffect();

		g_hdrBuffer.Read(GL_TEXTURE0);
		RenderQuad();
		g_currentEffect->UnUseEffect();

	}

	void ForwardShading()
	{
		g_hdrBuffer.Write(
			[] {
				// 3 shadow map pass, 1 pbr pass, 1 cubemap pass
				for (size_t i = 0; i < g_dataRenderingByGraphicThread->g_renderPasses.size(); ++i)
				{
					// Update current render pass
					g_currentRenderPass = i;
					// Update frame data
					g_uniformBufferMap[UBT_Frame].Update(&g_dataRenderingByGraphicThread->g_renderPasses[i].FrameData);
					// Execute pass function
					g_dataRenderingByGraphicThread->g_renderPasses[i].RenderPassFunction();
				}

				// Render spheres
#ifdef ENABLE_CLOTH_SIM
				if (ClothSim::g_bDrawNodes)
				{
					g_currentEffect = GetEffectByKey(EET_DrawDebugCircles);
					g_currentEffect->UseEffect();
					g_uniformBufferMap[UBT_Frame].Update(&g_dataRenderingByGraphicThread->g_renderPasses[3].FrameData);
					for (size_t i = 0; i < ClothSim::VC; ++i)
					{
						cMesh* _Point = cMesh::s_manager.Get(s_point);
						cTransform _tempTransform;

						g_currentEffect->SetFloat("radius", 5 * 5 / static_cast<float>(CLOTH_RESOLUTION));
						glm::vec3 sphereColor = glm::vec3(1.0, 0.0, 0.0);
						if (ClothSim::g_particles[i].isFixed)
							sphereColor = glm::vec3(0, 1, 0);

						g_currentEffect->SetVec3("color", sphereColor);
						_tempTransform.SetPosition(g_dataRenderingByGraphicThread->particles[i]);
						_tempTransform.Update();
						g_uniformBufferMap[UBT_Drawcall].Update(&UniformBufferFormats::sDrawCall(_tempTransform.M(), _tempTransform.TranspostInverse()));
						_Point->Render();
					}
					g_currentEffect->UnUseEffect();
				}
#endif // ENABLE_CLOTH_SIM

			}
		);


	}

	void GBuffer_Pass()
	{
		g_currentEffect = GetEffectByKey(EET_GBuffer);
		g_currentEffect->UseEffect();

		glClearColor(s_clearColor.r, s_clearColor.g, s_clearColor.b, 0.f);
		// A lot of things can be cleaned like color buffer, depth buffer, so we need to specify what to clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		RenderScene(g_currentEffect);
		g_currentEffect->UnUseEffect();
	}

	void DisplayGBuffer_Pass()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		g_currentEffect = GetEffectByKey(EET_GBufferDisplay);
		g_currentEffect->UseEffect();
		const ERenderMode& renderMode = g_dataRenderingByGraphicThread->g_renderMode;
		g_currentEffect->SetInteger("displayMode", static_cast<GLint>(renderMode) - 2);
		GLenum _textureUnits[4] = { GL_TEXTURE0 , GL_TEXTURE1 ,GL_TEXTURE2, GL_TEXTURE3 };
		g_GBuffer.Read(_textureUnits);
		g_ssao_blur_Buffer.Read(GL_TEXTURE4);
		RenderQuad(g_dataRenderingByGraphicThread->g_renderPasses[g_currentRenderPass].FrameData);
		g_currentEffect->UnUseEffect();
	}

	void Deferred_Lighting_Pass()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		g_currentEffect = GetEffectByKey(EET_DeferredLighting);
		g_currentEffect->UseEffect();

		GLenum _textureUnits[4] = { GL_TEXTURE0 , GL_TEXTURE1 ,GL_TEXTURE2, GL_TEXTURE3 };
		g_GBuffer.Read(_textureUnits);
		g_ssao_blur_Buffer.Read(GL_TEXTURE0 + SHADOWMAP_START_TEXTURE_UNIT + OMNI_SHADOW_MAP_COUNT * 2 + 1);
		UpdateInfoForPBR();

		RenderQuad(g_dataRenderingByGraphicThread->g_renderPasses[g_currentRenderPass].FrameData);

		g_currentEffect->UnUseEffect();
	}

	void DeferredShading()
	{
		/** 0. Update lighting data pass 0-2*/
		for (size_t i = 0; i < 3; ++i)
		{
			// Update current render pass
			g_currentRenderPass = i;
			// Update frame data
			g_uniformBufferMap[UBT_Frame].Update(&g_dataRenderingByGraphicThread->g_renderPasses[i].FrameData);
			// Execute pass function
			g_dataRenderingByGraphicThread->g_renderPasses[i].RenderPassFunction();
		}
		/** 1. Capture the whole scene with PBR material*/		
		Profiler::StartRecording(Profiler::EPT_GBuffer);
		g_GBuffer.Write(
			[] {
				g_currentRenderPass = 3; // PBR pass
				g_uniformBufferMap[UBT_Frame].Update(&g_dataRenderingByGraphicThread->g_renderPasses[g_currentRenderPass].FrameData);
				GBuffer_Pass();
			});		
		Profiler::StopRecording(Profiler::EPT_GBuffer);
		/** 2.1 Capture SSAO buffer*/
		g_ssaoBuffer.Write(
			[] {
				g_currentEffect = GetEffectByKey(EET_SSAO);
				g_currentEffect->UseEffect();
				glClear(GL_COLOR_BUFFER_BIT);

				g_GBuffer.ReadNormalRoughness(GL_TEXTURE0);
				g_GBuffer.ReadDepth(GL_TEXTURE1);
				cTexture* _noiseTex = cTexture::s_manager.Get(g_ssaoNoiseTexture);
				_noiseTex->UseTexture(GL_TEXTURE2);

				RenderQuad(g_dataRenderingByGraphicThread->g_renderPasses[g_currentRenderPass].FrameData);

				g_currentEffect->UnUseEffect();
			}
		);
		/** 2.2 Capture SSAO_blur buffer*/
		g_ssao_blur_Buffer.Write(
			[] {
				g_currentEffect = GetEffectByKey(EET_SSAO_Blur);
				g_currentEffect->UseEffect();
				glClear(GL_COLOR_BUFFER_BIT);

				g_ssaoBuffer.Read(GL_TEXTURE0);
				RenderQuad(g_dataRenderingByGraphicThread->g_renderPasses[g_currentRenderPass].FrameData);

				g_currentEffect->UnUseEffect();
			}
		);
		/** 3.1B. Display GBuffer alternatively */
		if (g_dataRenderingByGraphicThread->g_renderMode != ERM_DeferredShading)
			DisplayGBuffer_Pass();
		else
		{
			/** 3. Capture HDR buffer*/
			g_hdrBuffer.Write(
				[] {
					/** 3.1A. Show the deferred shading */
					Profiler::StartRecording(Profiler::EPT_DeferredLighting);
					Deferred_Lighting_Pass();
					Profiler::StopRecording(Profiler::EPT_DeferredLighting);
					/** 3.2. Display cubemap at the end */
					{
						glBlitNamedFramebuffer(
							g_GBuffer.fbo(), g_hdrBuffer.fbo(), 0, 0, g_GBuffer.GetWidth(), g_GBuffer.GetHeight(), 0, 0, g_GBuffer.GetWidth(), g_GBuffer.GetHeight(), GL_DEPTH_BUFFER_BIT, GL_NEAREST
						);
						g_currentRenderPass = 4; // Cubemap pass
						g_uniformBufferMap[UBT_Frame].Update(&g_dataRenderingByGraphicThread->g_renderPasses[g_currentRenderPass].FrameData);
						CubeMap_Pass();
					}
				}
			);

		}
	}

	void Gizmo_DrawOutline(const cModel* i_outlineModel, const cTransform& i_modelTransform)
	{
		g_uniformBufferMap[UBT_Drawcall].Update(&UniformBufferFormats::sDrawCall(i_modelTransform.M(), i_modelTransform.TranspostInverse()));

		glEnable(GL_STENCIL_TEST);
		glDisable(GL_DEPTH_TEST);
		{
			// Get stencil buffer to the g_outline buffer
			g_outlineBuffer.Write([&]
				{
					g_currentEffect = GetEffectByKey(EET_Unlit);
					g_currentEffect->UseEffect();

					glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

					glStencilFunc(GL_ALWAYS, 1, 0xFF);
					glStencilMask(0xFF);
					// Draw, need the stencil buffer info only
					i_outlineModel->RenderWithoutMaterial();

					g_currentEffect->UnUseEffect();
				}
			);

			// draw the outline now
			g_currentEffect = GetEffectByKey(EET_Outline);
			g_currentEffect->UseEffect();

			// Copy stencil buffer data to the default frame buffer
			glBlitNamedFramebuffer(g_outlineBuffer.fbo(), 0, 0, 0, g_outlineBuffer.GetWidth(), g_outlineBuffer.GetHeight(), 0, 0, g_outlineBuffer.GetWidth(), g_outlineBuffer.GetHeight(), GL_STENCIL_BUFFER_BIT, GL_NEAREST);

			glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
			glStencilMask(0x00);

			i_outlineModel->RenderWithoutMaterial();

			glStencilMask(0xFF);
			glStencilFunc(GL_ALWAYS, 1, 0xFF);
			g_currentEffect->UnUseEffect();
		}
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);


	}
	void Gizmo_DrawDebugCircle(float i_radius, const Color& i_color, const glm::vec3& i_position)
	{
		g_currentEffect = GetEffectByKey(EET_DrawDebugCircles);
		g_currentEffect->UseEffect();

		cMesh* _Point = cMesh::s_manager.Get(s_point);
		g_currentEffect->SetFloat("radius", i_radius);

		g_currentEffect->SetVec3("color", static_cast<glm::vec3>(i_color));
		cTransform tempTr(i_position, glm::quat(1, 0, 0, 0), glm::vec3(i_radius));
		g_uniformBufferMap[UBT_Drawcall].Update(&UniformBufferFormats::sDrawCall(tempTr.M(), tempTr.TranspostInverse()));
		_Point->Render();

		g_currentEffect->UnUseEffect();
	}

	void Gizmo_RenderSelectingTransform(const cTransform* i_arrowTransforms)
	{
		glClear(GL_DEPTH_BUFFER_BIT);

		g_currentEffect = GetEffectByKey(EET_Unlit);
		g_currentEffect->UseEffect();
		int hoveredArrowDirection = -1;
		IsTransformGizmoIsHovered(g_dataGetFromRenderThread->g_selectionID, hoveredArrowDirection);
		for (int i = 0; i < 3; ++i)
		{
			g_uniformBufferMap[UBT_Drawcall].Update(&UniformBufferFormats::sDrawCall(i_arrowTransforms[i].M(), i_arrowTransforms[i].TranspostInverse()));
			if (i == hoveredArrowDirection)
			{
				g_currentEffect->SetVec3("unlitColor", 0.8f * glm::vec3(g_outlineColor.r, g_outlineColor.g, g_outlineColor.b));
				g_arrowHandles[i].RenderWithoutMaterial();
			}
			else
				g_arrowHandles[i].Render();
		}
		g_currentEffect->UnUseEffect();
	}

	void Gizmo_DrawDebugCaptureVolume() {

		g_currentEffect = GetEffectByKey(EET_DrawDebugCircles);
		g_currentEffect->UseEffect();
		auto _capturesRef = EnvironmentCaptureManager::GetCapturesReferences();
		for (size_t i = 0; i < _capturesRef.size(); ++i)
		{
			cMesh* _Point = cMesh::s_manager.Get(s_point);
			const cSphere& _outerBV = _capturesRef[i]->BV;
			const cSphere& _innerBV = _capturesRef[i]->InnerBV;
			cTransform _tempTransform;

			// outer
			g_currentEffect->SetFloat("radius", _outerBV.r());
			g_currentEffect->SetVec3("color", glm::vec3(1, 1, 1));
			_tempTransform.SetPosition(_outerBV.c());
			_tempTransform.Update();
			g_uniformBufferMap[UBT_Drawcall].Update(&UniformBufferFormats::sDrawCall(_tempTransform.M(), _tempTransform.TranspostInverse()));
			_Point->Render();

			// inner
			g_currentEffect->SetFloat("radius", _innerBV.r());
			g_currentEffect->SetVec3("color", glm::vec3(1, 0, 0));
			_tempTransform.SetPosition(_innerBV.c());
			_tempTransform.Update();
			g_uniformBufferMap[UBT_Drawcall].Update(&UniformBufferFormats::sDrawCall(_tempTransform.M(), _tempTransform.TranspostInverse()));
			_Point->Render();
		}
		g_currentEffect->UnUseEffect();
	}

	void Gizmo_RenderVertexNormal()
	{
		g_currentEffect = GetEffectByKey(EET_NormalDisplay);
		g_currentEffect->UseEffect();

		glClear(GL_DEPTH_BUFFER_BIT);
		RenderScene(nullptr);
		g_currentEffect->UnUseEffect();
	}

	void Gizmo_RenderTriangulation()
	{
		sWindowInput* _input = Application::GetCurrentApplication()->GetCurrentWindow()->GetWindowInput();
		if (!_input->IsKeyDown(GLFW_KEY_T)) return;

		g_currentEffect = GetEffectByKey(EET_TriangulationDisplay);
		g_currentEffect->UseEffect();

		glClear(GL_DEPTH_BUFFER_BIT);
		RenderScene(nullptr);
		g_currentEffect->UnUseEffect();
	}

	void SelctionBuffer_Pass(const std::vector<std::pair<cModel, cTransform>>& i_renderMap, bool i_needRenderTransformGizmo)
	{
		glDisable(GL_CULL_FACE);
		g_currentEffect = GetEffectByKey(EET_SelectionBuffer);
		g_currentEffect->UseEffect();

		g_selectionBuffer.Write([&]
			{
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				bool depthCleared = false;
				for (int i = 0; i < i_renderMap.size(); ++i)
				{
					// when it is rendering the transform, should clear the depth first
					if (i_needRenderTransformGizmo && i >= i_renderMap.size() - 3 && !depthCleared)
					{
						depthCleared = true;
						glClear(GL_DEPTH_BUFFER_BIT);
					}

					auto* it = &i_renderMap[i];
					// 1. Update draw call data
					g_uniformBufferMap[UBT_Drawcall].Update(&UniformBufferFormats::sDrawCall(it->second.M(), it->second.TranspostInverse()));
					// 2. Draw
					uint32_t modelID = it->first.SelectableID;
					int r = (modelID & 0x000000FF) >> 0;
					int g = (modelID & 0x0000FF00) >> 8;
					int b = (modelID & 0x00FF0000) >> 16;
					g_currentEffect->SetVec3("selectionColor", glm::vec3(r / 255.f, g / 255.f, b / 255.f));

					it->first.RenderWithoutMaterial();
				}
			});
		g_currentEffect->UnUseEffect();

		// bind the frame buffer and ready to read pixel from the texture
		int _prevFbo;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_prevFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, g_selectionBuffer.fbo());

		/*
				glFlush();
				glFinish();*/
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		unsigned char data[3] = { 0 };
		sIO& _IO = g_dataRenderingByGraphicThread->g_IO;
		int mouseX = glm::clamp(static_cast<int>(_IO.MousePos.x), 0, Application::GetCurrentApplication()->GetCurrentWindow()->GetBufferWidth());
		int mouseY = glm::clamp(static_cast<int>(Application::GetCurrentApplication()->GetCurrentWindow()->GetBufferHeight() - _IO.MousePos.y), 0, Application::GetCurrentApplication()->GetCurrentWindow()->GetBufferHeight());
		glReadPixels(mouseX, mouseY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, data);
		uint32_t pickedID = data[0] + data[1] * 256 + data[2] * 256 * 256;
		g_dataGetFromRenderThread->g_selectionID = pickedID;

		glBindFramebuffer(GL_FRAMEBUFFER, _prevFbo);
		glEnable(GL_CULL_FACE);
		assert(GL_NO_ERROR == glGetError());
	}

	void EditorPass()
	{
		// current selected object in application 
		const auto* frameData = &g_dataRenderingByGraphicThread->g_renderPasses[3].FrameData;
		auto& selectionID = g_dataRenderingByGraphicThread->g_selectingItemID;
		cTransform arrowTransforms[3];
		bool needRenderTarnsformGizmo = false;
		auto renderMapForSelectionBuffer = g_dataRenderingByGraphicThread->g_renderPasses[3].ModelToTransform_map; // original render map
		for (auto item : g_dataRenderingByGraphicThread->g_pointLights)
		{
			g_quadHandle.SelectableID = item.SelectableID;
			cTransform pLightTransform(item.Transform.Position(), glm::quatLookAt(glm::normalize(frameData->GetViewPosition() - item.Transform.Position()), glm::vec3(frameData->ViewMatrix[0][0], frameData->ViewMatrix[0][1], frameData->ViewMatrix[0][2])), glm::vec3(10, 10, 10));
			renderMapForSelectionBuffer.push_back({ g_quadHandle, pLightTransform });
		}
		// update frame data, from the editor camera
		g_uniformBufferMap[UBT_Frame].Update(frameData);

		cTransform* selectableTransform = nullptr;
		if (ISelectable::IsValid(selectionID))
		{
			// only draw the outline when the model has owner
			if ((needRenderTarnsformGizmo = ISelectable::s_selectableList[selectionID]->GetBoundTransform(selectableTransform)))
			{
				const cModel* _model = dynamic_cast<cModel*>(ISelectable::s_selectableList[selectionID]);
				// if it is a model
				if (_model)
					Gizmo_DrawOutline(_model, *selectableTransform);
				const cPointLight* _Light = dynamic_cast<cPointLight*>(ISelectable::s_selectableList[selectionID]);
				// if it is a light
				if (_Light)
					Gizmo_DrawDebugCircle(_Light->Range, Color(1,1,0), _Light->Transform.Position());

				// Calculate the transform gizmo's transform and add three arrow models to the render map
				{
					// Forward
					arrowTransforms[0].SetRotation(selectableTransform->Rotation() * glm::quat(glm::vec3(glm::radians(90.f), 0, 0)));
					// Right									  
					arrowTransforms[1].SetRotation(selectableTransform->Rotation() * glm::quat(glm::vec3(0, 0, glm::radians(90.f))));
					// Up											
					arrowTransforms[2].SetRotation(selectableTransform->Rotation() * glm::quat(glm::vec3(0, glm::radians(90.f), 0)));

					// Get scale according to the camera
					float distToCamera = glm::distance(g_dataRenderingByGraphicThread->g_renderPasses[3].FrameData.GetViewPosition(), selectableTransform->Position());
					constexpr float scaleOneDistance = 300.f;
					float _scale = distToCamera / scaleOneDistance;

					// Set position and scale
					for (int i = 0; i < 3; ++i)
					{
						arrowTransforms[i].SetPosition(selectableTransform->Position());
						arrowTransforms[i].SetScale(glm::vec3(_scale));
						arrowTransforms[i].Update();
						renderMapForSelectionBuffer.push_back({ g_arrowHandles[i], arrowTransforms[i] });
					}

				}
			}
		}

		// Handle selection
		Profiler::StartRecording(Profiler::EPT_Selection);
		SelctionBuffer_Pass(renderMapForSelectionBuffer, needRenderTarnsformGizmo);
		Profiler::StopRecording(Profiler::EPT_Selection);

		if (ISelectable::IsValid(selectionID) && selectableTransform)
		{
			Gizmo_RenderSelectingTransform(arrowTransforms);
		}

	}

	bool IsTransformGizmoIsHovered(uint32_t i_selectionID, int& o_direction)
	{
		bool result = false;
		if (ImGui::GetIO().WantCaptureMouse) return false;
		for (int i = 0; i < 3; ++i)
		{
			if (g_arrowHandles[i].SelectableID == i_selectionID)
			{
				result = true;
				o_direction = i;
				break;
			}
		}
		return result;
	}

	void SetArrowBeingSelected(bool i_selected, int i_direction)
	{
		if (i_direction >= 0 && i_direction < 3)
		{
			cMatUnlit* _arrowMat = dynamic_cast<cMatUnlit*>(cMaterial::s_manager.Get(g_arrowHandles[i_direction].GetMaterialAt()));
			if (_arrowMat)
			{
				_arrowMat->SetUnlitColor(i_selected ? g_outlineColor * 0.8f : g_arrowColor[i_direction]);
			}
		}
	}


}