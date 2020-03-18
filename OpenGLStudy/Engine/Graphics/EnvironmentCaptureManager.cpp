#pragma once
#include "Graphics/EnvironmentCaptureManager.h"
#include "UniformBuffer/UniformBuffer.h"
#include "RenderStructs.h"
#include <stdio.h>
#include <vector>
#include "Graphics/Effect/Effect.h"
#include "Graphics/Graphics.h"
#include "Application/Application.h"
#include "Application/Window/Window.h"

namespace Graphics
{
	namespace EnvironmentCaptureManager
	{
		// Constant data
		// ------------------------------------------------------------------------------------------------------------------------------------
		const int g_MaximumOverlapVolumes = 4; // Only allow maximum 4 spheres overlap with the POI (point of interests)
		const int g_MaximumCaptureProbesInAScene = 10;
		const GLuint g_IrradianceMapResolution = 32;
		const GLuint g_PrefilterMapResolution = 256;
		// Global data
		// ------------------------------------------------------------------------------------------------------------------------------------

		cUniformBuffer s_uniformBuffer_EnvCaptureWeight(eUniformBufferType::UBT_EnvCaptureWeight);
		std::vector<sCaptureProbes> g_CaptureProbesList; // Storing the capture probes

		bool Initialize()
		{
			auto result = true;

			if (result = s_uniformBuffer_EnvCaptureWeight.Initialize(nullptr)) {
				s_uniformBuffer_EnvCaptureWeight.Bind();
			}
			else
			{
				printf("Fail to initialize uniformBuffer_EnvCaptureWeight. \n");
				return result;
			}

			// Allocate memory for capture probes
			g_CaptureProbesList.reserve(g_MaximumCaptureProbesInAScene);


			return result;
		}

		bool CleanUp()
		{
			auto result = true;
			if (!(result = s_uniformBuffer_EnvCaptureWeight.CleanUp())) {
				printf("Fail to cleanup uniformBuffer_EnvCaptureWeight\n");
			}

			for (size_t i = 0; i < g_CaptureProbesList.size(); ++i)
			{
				g_CaptureProbesList[i].CleanUp();
			}
			g_CaptureProbesList.clear();

			return result;
		}

		bool AddCaptureProbes(const glm::vec3& i_position, GLfloat i_radius, GLuint i_environmentCubemapSize)
		{
			auto result = true;
			if (g_CaptureProbesList.size() >= g_MaximumCaptureProbesInAScene) {
				result = false;
				printf("No more capture probe can be added to the scene because it reaches the capacity cap.\n");
				return result;
			}

			g_CaptureProbesList.push_back(sCaptureProbes(i_position, i_radius, i_environmentCubemapSize));
			GLuint _lastIdx = g_CaptureProbesList.size() - 1;
			assert(i_environmentCubemapSize > 512);

			if (!(result = g_CaptureProbesList[_lastIdx].EnvironmentProbe.Initialize(i_radius, i_environmentCubemapSize, i_environmentCubemapSize, ETT_FRAMEBUFFER_HDR_MIPMAP_CUBEMAP, i_position)))
			{
				printf("Fail to create environment probe.\n");
				return result;
			}
			if (!(result = g_CaptureProbesList[_lastIdx].IrradianceProbe.Initialize(i_radius, g_IrradianceMapResolution, g_IrradianceMapResolution, ETT_FRAMEBUFFER_HDR_CUBEMAP, i_position)))
			{
				printf("Fail to create Irradiance probe.\n");
				return result;
			}
			if (!(result = g_CaptureProbesList[_lastIdx].PrefilterProbe.Initialize(i_radius, g_PrefilterMapResolution, g_PrefilterMapResolution, ETT_FRAMEBUFFER_HDR_MIPMAP_CUBEMAP, i_position)))
			{
				printf("Fail to create Pre-filter probe.\n");
				return result;
			}
		}

		void CaptureEnvironment(Graphics::sDataRequiredToRenderAFrame* i_renderThreadData)
		{
			Graphics::cEffect* _currentEffct = nullptr;
			Graphics::cUniformBuffer* _uniformBuffer_frame = Graphics::GetUniformBuffer(UBT_Frame);
			for (size_t k = 0; k < g_CaptureProbesList.size(); ++k)
			{
				constexpr GLuint shadowmapPassesCount = 3; // point, spot, directional
				// i. First deal with existing shadow maps
				{
					for (size_t i = 0; i < shadowmapPassesCount; ++i)
					{
						int _currentPass = i;
						Graphics::SetCurrentPass(_currentPass);
						_uniformBuffer_frame->Update(&i_renderThreadData->s_renderPasses[_currentPass].FrameData);
						// Execute pass function
						i_renderThreadData->s_renderPasses[_currentPass].RenderPassFunction();
					}
				}

				// ii. start to capture the environment cube map
				{
					g_CaptureProbesList[k].EnvironmentProbe.StartCapture();

					GLuint passesPerFace = (i_renderThreadData->s_renderPasses.size() - shadowmapPassesCount) / 6.f;
					for (size_t i = 0; i < 6; ++i)
					{
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, g_CaptureProbesList[k].EnvironmentProbe.GetCubemapTextureID(), 0);
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
						for (size_t j = 0; j < passesPerFace; ++j)
						{
							// Update current render pass
							int _currentRenderPass = i * passesPerFace + j + shadowmapPassesCount;
							Graphics::SetCurrentPass(_currentRenderPass);
							// Update frame data using the environment probes' projection and view matrix
							Graphics::UniformBufferFormats::sFrame _frame(g_CaptureProbesList[k].EnvironmentProbe.GetProjectionMat4(), g_CaptureProbesList[k].EnvironmentProbe.GetViewMat4(i));
							_frame.ViewPosition = g_CaptureProbesList[k].BV.c();
							_uniformBuffer_frame->Update(&_frame);
							// Execute pass function
							i_renderThreadData->s_renderPasses[_currentRenderPass].RenderPassFunction();
						}

					}
					g_CaptureProbesList[k].EnvironmentProbe.StopCapture();

					// After capturing the scene, generate the mip map by opengl itself
					glBindTexture(GL_TEXTURE_CUBE_MAP, g_CaptureProbesList[k].EnvironmentProbe.GetCubemapTextureID());
					glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
					glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
				}

				// iii. start to capture the irradiance map
				{
					glFrontFace(GL_CW);
					_currentEffct = Graphics::GetEffectByKey("IrradConvolution");
					_currentEffct->UseEffect();

					_currentEffct->SetInteger("cubemapTex", 0);
					_currentEffct->ValidateProgram();
					cTexture* _envProbeTexture = cTexture::s_manager.Get(g_CaptureProbesList[k].EnvironmentProbe.GetCubemapTextureHandle());
					_envProbeTexture->UseTexture(GL_TEXTURE0);

					g_CaptureProbesList[k].IrradianceProbe.StartCapture();
					for (size_t i = 0; i < 6; ++i)
					{
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, g_CaptureProbesList[k].IrradianceProbe.GetCubemapTextureID(), 0);
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

						UniformBufferFormats::sFrame _cubemapFrameData(g_CaptureProbesList[k].IrradianceProbe.GetProjectionMat4(), g_CaptureProbesList[k].IrradianceProbe.GetViewMat4(i));
						_cubemapFrameData.ViewPosition = g_CaptureProbesList[k].IrradianceProbe.GetPosition();
						_uniformBuffer_frame->Update(&_cubemapFrameData);

						// Render cube
						cModel* _cube = cModel::s_manager.Get(Graphics::GetPrimitive(EPT_Cube));
						if (_cube) { _cube->RenderWithoutMaterial(); }
					}
					g_CaptureProbesList[k].IrradianceProbe.StopCapture();

					_currentEffct->UnUseEffect();

				}

				// iv. start to capture the pre-filter cube map
				{
					_currentEffct = Graphics::GetEffectByKey("CubemapPrefilter");
					_currentEffct->UseEffect();

					_currentEffct->SetInteger("cubemapTex", 0);
					_currentEffct->SetInteger("resolution", g_CaptureProbesList[k].EnvironmentProbe.GetWidth());

					_currentEffct->ValidateProgram();
					cTexture* _envProbeTexture = cTexture::s_manager.Get(g_CaptureProbesList[k].EnvironmentProbe.GetCubemapTextureHandle());
					_envProbeTexture->UseTexture(GL_TEXTURE0);

					g_CaptureProbesList[k].PrefilterProbe.StartCapture();

					constexpr GLuint maxMipLevels = 5;
					for (GLuint i = maxMipLevels; i > 0; --i) // capture multiple mip-map levels from low res to high res
					{
						GLuint mip = i - 1;
						// resize frame buffer according to mip-level size.
						GLuint mipWidth = g_CaptureProbesList[k].PrefilterProbe.GetWidth() * glm::pow(0.5f, mip);
						GLuint mipHeight = g_CaptureProbesList[k].PrefilterProbe.GetHeight() * glm::pow(0.5f, mip);

						glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight); // resize render buffer too
						Application::cApplication* _app = Application::GetCurrentApplication();
						if (_app) { _app->GetCurrentWindow()->SetViewportSize(mipWidth, mipHeight); }

						float roughness = static_cast<float>(mip) / static_cast<float>(maxMipLevels - 1);
						_currentEffct->SetFloat("roughness", roughness);

						for (size_t i = 0; i < 6; ++i)
						{
							glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, g_CaptureProbesList[k].PrefilterProbe.GetCubemapTextureID(), mip);
							glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
							UniformBufferFormats::sFrame _cubemapFrameData(g_CaptureProbesList[k].PrefilterProbe.GetProjectionMat4(), g_CaptureProbesList[k].PrefilterProbe.GetViewMat4(i));
							_cubemapFrameData.ViewPosition = g_CaptureProbesList[k].PrefilterProbe.GetPosition();
							_uniformBuffer_frame->Update(&_cubemapFrameData);

							// Render cube
							cModel* _cube = cModel::s_manager.Get(Graphics::GetPrimitive(EPT_Cube));
							if (_cube) { _cube->RenderWithoutMaterial(); }
						}
					}

					g_CaptureProbesList[k].PrefilterProbe.StopCapture();
					_currentEffct->UnUseEffect();
					glFrontFace(GL_CCW);
				}
			}
		}

		void UpdatePointOfInterest(const glm::vec3& i_position)
		{

		}

		const Graphics::EnvironmentCaptureManager::sCaptureProbes& GetCaptureProbesAt(int i_idx)
		{
			assert(i_idx < g_CaptureProbesList.size());

			return g_CaptureProbesList[i_idx];
		}

	}
}