#include "Graphics/Graphics.h"
#include "Graphics/EnvironmentCaptureManager.h"
#include "Material/Unlit/MatUnlit.h"
#include "Material/PBR_MR/MatPBRMR.h"
#include "Application/Window/WindowInput.h"
#include "Application/Application.h"
#include "Application/Window/Window.h"
#include "Graphics/FrameBuffer/GeometryBuffer.h"

namespace Graphics
{
	extern unsigned int s_currentRenderPass;
	extern cEffect* s_currentEffect;
	extern sDataRequiredToRenderAFrame* s_dataRenderingByGraphicThread;
	extern 	std::map<eUniformBufferType, cUniformBuffer> g_uniformBufferMap;
	extern UniformBufferFormats::sLighting s_globalLightingData;
	extern cFrameBuffer s_brdfLUTTexture;
	extern 	cFrameBuffer g_hdrBuffer;
	extern cGBuffer g_GBuffer;
	extern cModel::HANDLE s_cubeHandle;
	extern cModel::HANDLE s_arrowHandle;
	extern cModel::HANDLE s_quadHandle;
	extern cMesh::HANDLE s_point;

	// clear color
	Color s_clearColor;
	// arrow colors
	Color s_arrowColor[3] = { Color(0, 0, 0.8f), Color(0.8f, 0, 0),Color(0, 0.8f, 0) };

	void RenderQuad()
	{
		UniformBufferFormats::sFrame defaultFrameData;
		g_uniformBufferMap[UBT_Frame].Update(&defaultFrameData);
		
		UniformBufferFormats::sDrawCall defaultDrawcallData;
		g_uniformBufferMap[UBT_Drawcall].Update(&defaultDrawcallData);

		// Render quad
		cModel* _quad = cModel::s_manager.Get(s_quadHandle);
		if (_quad) {
			_quad->RenderWithoutMaterial();
		}
	}

	void UpdateLightingData()
	{
		s_dataRenderingByGraphicThread->s_ambientLight.Illuminate();

		for (size_t i = 0; i < s_dataRenderingByGraphicThread->s_spotLights.size(); ++i)
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

		for (size_t i = 0; i < s_dataRenderingByGraphicThread->s_pointLights.size(); ++i)
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
		g_uniformBufferMap[UBT_Lighting].Update(&s_globalLightingData);

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
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				// Draw scenes
				RenderScene(nullptr);

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

		for (size_t i = 0; i < s_dataRenderingByGraphicThread->s_pointLights.size(); ++i)
		{
			auto* it = &s_dataRenderingByGraphicThread->s_pointLights[i];
			cFrameBuffer* _pointLightFBO = it->GetShadowMap();
			if (_pointLightFBO) {

				// for each light, needs to update the frame data
				Graphics::UniformBufferFormats::sFrame _frameData_PointLightShadow;
				g_uniformBufferMap[UBT_Frame].Update(&_frameData_PointLightShadow);

				// point need extra uniform variables to be passed in to shader
				it->SetupLight(s_currentEffect->GetProgramID(), i);
				it->SetLightUniformTransform();
				// write buffer to the texture
				_pointLightFBO->Write();
				assert(glGetError() == GL_NO_ERROR);

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				// Draw scenes
				RenderScene(nullptr);

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

		for (size_t i = 0; i < s_dataRenderingByGraphicThread->s_spotLights.size(); ++i)
		{
			auto* it = &s_dataRenderingByGraphicThread->s_spotLights[i];

			cFrameBuffer* _spotLightFB = it->GetShadowMap();
			if (_spotLightFB) {

				// for each light, needs to update the frame data
				Graphics::UniformBufferFormats::sFrame _frameData_SpotLightShadow(it->GetProjectionmatrix(), it->GetViewMatrix());

				g_uniformBufferMap[UBT_Frame].Update(&_frameData_SpotLightShadow);

				// write buffer to the texture
				_spotLightFB->Write();
				assert(glGetError() == GL_NO_ERROR);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				// Draw scenes
				RenderScene(nullptr);
				// switch back to original buffer
				_spotLightFB->UnWrite();
				assert(glGetError() == GL_NO_ERROR);
			}
		}
		s_currentEffect->UnUseEffect();
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}

	void BlinnPhong_Pass()
	{
		s_currentEffect = GetEffectByKey(EET_BlinnPhong);
		s_currentEffect->UseEffect();
		glClearColor(s_clearColor.r, s_clearColor.g, s_clearColor.b, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Update Lighting Data
		UpdateLightingData();
		// Start a draw call loop
		RenderScene(s_currentEffect);
		s_currentEffect->UnUseEffect();
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
		for (size_t i = currentReadyCapturesCount; i < maxCubemapMixing; ++i)
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
		for (size_t i = 0; i < sphereOffset; ++i)
		{
			// 1. Update draw call data
			g_uniformBufferMap[UBT_Drawcall].Update(&UniformBufferFormats::sDrawCall(renderList[i].second.M(), renderList[i].second.TranspostInverse()));
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
				g_uniformBufferMap[UBT_Drawcall].Update(&UniformBufferFormats::sDrawCall(renderList[i * 5 + j + sphereOffset].second.M(), renderList[i * 5 + j + sphereOffset].second.TranspostInverse()));
				// 2. Draw
				cModel* _model = cModel::s_manager.Get(renderList[i * 5 + j + sphereOffset].first);
				if (_model) {
					Graphics::cMatPBRMR* _sphereMat = dynamic_cast<Graphics::cMatPBRMR*>(_model->GetMaterialAt());
					_sphereMat->UpdateMetalnessIntensity(1.f / 4.f * j );
					_sphereMat->UpdateRoughnessIntensity(1.05f - 1.f / 4.f *i);
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
		RenderScene(s_currentEffect);
		s_currentEffect->UnUseEffect();

		// set depth function back to default
		glEnable(GL_CULL_FACE);
	}

	void HDR_Pass()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		s_currentEffect = GetEffectByKey(EET_HDREffect);
		s_currentEffect->UseEffect();

		g_hdrBuffer.Read(GL_TEXTURE0);
		RenderQuad();
		s_currentEffect->UnUseEffect();
	}

	void GBuffer_Pass()
	{
		s_currentEffect = GetEffectByKey(EET_GBuffer);
		s_currentEffect->UseEffect();

		glClearColor(s_clearColor.r, s_clearColor.g, s_clearColor.b, 0.f);
		// A lot of things can be cleaned like color buffer, depth buffer, so we need to specify what to clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		auto& renderList = s_dataRenderingByGraphicThread->s_renderPasses[s_currentRenderPass].ModelToTransform_map;
		const auto sphereOffset = renderList.size() - 25;
		// draw the space first
		for (size_t i = 0; i < sphereOffset; ++i)
		{
			// 1. Update draw call data
			g_uniformBufferMap[UBT_Drawcall].Update(&UniformBufferFormats::sDrawCall(renderList[i].second.M(), renderList[i].second.TranspostInverse()));
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
				g_uniformBufferMap[UBT_Drawcall].Update(&UniformBufferFormats::sDrawCall(renderList[i * 5 + j + sphereOffset].second.M(), renderList[i * 5 + j + sphereOffset].second.TranspostInverse()));
				// 2. Draw
				cModel* _model = cModel::s_manager.Get(renderList[i * 5 + j + sphereOffset].first);
				if (_model) {
					Graphics::cMatPBRMR* _sphereMat = dynamic_cast<Graphics::cMatPBRMR*>(_model->GetMaterialAt());
					_sphereMat->UpdateMetalnessIntensity(1.f / 4.f * j);
					_sphereMat->UpdateRoughnessIntensity(1.05f - 1.f / 4.f *i);
					_model->UpdateUniformVariables(s_currentEffect->GetProgramID());
					_model->Render();
				}
			}
		}
		s_currentEffect->UnUseEffect();
	}

	void DisplayGBuffer_Pass()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		s_currentEffect = GetEffectByKey(EET_GBufferDisplay);		
		s_currentEffect->UseEffect();
		const ERenderMode& renderMode = s_dataRenderingByGraphicThread->g_renderMode;
		s_currentEffect->SetInteger("displayMode", static_cast<GLint>(renderMode) - 2);
		GLenum _textureUnits[4] = { GL_TEXTURE0 , GL_TEXTURE1 ,GL_TEXTURE2, GL_TEXTURE3 };
		g_GBuffer.Read(_textureUnits);
		RenderQuad();
		s_currentEffect->UnUseEffect();
	}

	void Deferred_Lighting_Pass()
	{
		return;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		s_currentEffect = GetEffectByKey(EET_DeferredLighting);
		s_currentEffect->UseEffect();

		GLenum _textureUnits[4] = { GL_TEXTURE0 , GL_TEXTURE1 ,GL_TEXTURE2, GL_TEXTURE3 };
		g_GBuffer.Read(_textureUnits);
		UpdateInfoForPBR();

		RenderQuad();

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
				g_uniformBufferMap[UBT_Drawcall].Update(&UniformBufferFormats::sDrawCall(arrowTransform[i].M(), arrowTransform[i].TranspostInverse()));

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
		for (size_t i = 0; i < _capturesRef.size(); ++i)
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
			g_uniformBufferMap[UBT_Drawcall].Update(&UniformBufferFormats::sDrawCall(_tempTransform.M(), _tempTransform.TranspostInverse()));
			_Point->Render();

			// inner
			s_currentEffect->SetFloat("radius", _innerBV.r());
			s_currentEffect->SetVec3("color", glm::vec3(1, 0, 0));
			_tempTransform.SetPosition(_innerBV.c());
			_tempTransform.Update();
			g_uniformBufferMap[UBT_Drawcall].Update(&UniformBufferFormats::sDrawCall(_tempTransform.M(), _tempTransform.TranspostInverse()));
			_Point->Render();
		}
		s_currentEffect->UnUseEffect();
	}

	void Gizmo_RenderVertexNormal()
	{
		s_currentEffect = GetEffectByKey(EET_NormalDisplay);
		s_currentEffect->UseEffect();

		glClear(GL_DEPTH_BUFFER_BIT);
		RenderScene(nullptr);
		s_currentEffect->UnUseEffect();
	}

	void Gizmo_RenderTriangulation()
	{
		sWindowInput* _input = Application::GetCurrentApplication()->GetCurrentWindow()->GetWindowInput();
		if (!_input->IsKeyDown(GLFW_KEY_T)) return;

		s_currentEffect = GetEffectByKey(EET_TriangulationDisplay);
		s_currentEffect->UseEffect();

		glClear(GL_DEPTH_BUFFER_BIT);
		RenderScene(nullptr);
		s_currentEffect->UnUseEffect();
	}
}