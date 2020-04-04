#pragma once
#pragma once
#include "Graphics/EnvironmentCaptureManager.h"
#include "UniformBuffer/UniformBuffer.h"
#include "RenderStructs.h"
#include <stdio.h>
#include <vector>
#include <algorithm>
#include "Graphics/Effect/Effect.h"
#include "Graphics/Graphics.h"
#include "Application/Application.h"
#include "Application/Window/Window.h"
#include "Cores/DataStructure/OctTree.h"

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
		const GLuint g_octTreeMaxVolumeWidth = 4096;
		// Global data
		// ------------------------------------------------------------------------------------------------------------------------------------

		cUniformBuffer g_uniformBuffer_EnvCaptureWeight(eUniformBufferType::UBT_EnvCaptureWeight);
		std::vector<sCaptureProbes> g_CaptureProbesList; // Storing the actual capture probes
		std::vector<sCaptureProbes*> g_CaptureProbesReferences;
		sOctTree g_EnvironmentCaptureAccelerationTree;


		// before update the POI, all elements in this array should be set to nullptr
		// depends on the mixing algorithm, element should point to different captures
		// after updating the POI, this array has the reference to all captures (maximum 4) that will be sent to the shader
		sCaptureProbes* g_capturesReadyToBeMixed[g_MaximumOverlapVolumes];
		// How many captures are valid in the g_capturesReadyToBeMixed array
		GLuint g_capturesReadyToBeMixedCount = 0;
		bool Initialize()
		{
			auto result = true;

			if (result = g_uniformBuffer_EnvCaptureWeight.Initialize(nullptr)) {
				g_uniformBuffer_EnvCaptureWeight.Bind();
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
			if (!(result = g_uniformBuffer_EnvCaptureWeight.CleanUp())) {
				printf("Fail to cleanup uniformBuffer_EnvCaptureWeight\n");
			}

			g_EnvironmentCaptureAccelerationTree.CleanUp();

			for (size_t i = 0; i < g_CaptureProbesList.size(); ++i)
			{
				g_CaptureProbesList[i].CleanUp();
			}
			g_CaptureProbesList.clear();

			return result;
		}

		bool AddCaptureProbes(const cSphere& i_outerSphere, float i_innerRadius, GLuint i_environmentCubemapSize)
		{
			auto result = true;
			if (g_CaptureProbesList.size() >= g_MaximumCaptureProbesInAScene) {
				result = false;
				printf("No more capture probe can be added to the scene because it reaches the capacity cap.\n");
				return result;
			}
			cSphere _innerSphere(i_outerSphere.c(), i_innerRadius);
			if (!(result = (i_outerSphere.Intersect(_innerSphere) == ECT_Contain)))
			{
				printf("Inner sphere is not contained by the outer sphere.\n");
				return result;
			}
			g_CaptureProbesList.push_back(sCaptureProbes(i_outerSphere, _innerSphere,1.0, i_environmentCubemapSize));
			GLuint _lastIdx = g_CaptureProbesList.size() - 1;
			assert(i_environmentCubemapSize > 512);

			if (!(result = g_CaptureProbesList[_lastIdx].EnvironmentProbe.Initialize(i_outerSphere.r(), i_environmentCubemapSize, i_environmentCubemapSize, ETT_FRAMEBUFFER_HDR_MIPMAP_CUBEMAP, i_outerSphere.c())))
			{
				printf("Fail to create environment probe.\n");
				return result;
			}
			if (!(result = g_CaptureProbesList[_lastIdx].IrradianceProbe.Initialize(i_outerSphere.r(), g_IrradianceMapResolution, g_IrradianceMapResolution, ETT_FRAMEBUFFER_HDR_CUBEMAP, i_outerSphere.c())))
			{
				printf("Fail to create Irradiance probe.\n");
				return result;
			}
			if (!(result = g_CaptureProbesList[_lastIdx].PrefilterProbe.Initialize(i_outerSphere.r(), g_PrefilterMapResolution, g_PrefilterMapResolution, ETT_FRAMEBUFFER_HDR_MIPMAP_CUBEMAP, i_outerSphere.c())))
			{
				printf("Fail to create Pre-filter probe.\n");
				return result;
			}

			g_CaptureProbesReferences.push_back(&g_CaptureProbesList[_lastIdx]);
			return result;
		}

		void BuildAccelerationStructure()
		{
			GLuint _halfWidth = g_octTreeMaxVolumeWidth / 2;
			glm::vec3 _posHalfDimension = glm::vec3(_halfWidth, _halfWidth, _halfWidth);

			g_EnvironmentCaptureAccelerationTree.InitializeTree(cBox(-_posHalfDimension, _posHalfDimension), g_CaptureProbesReferences);
		}

		const std::vector<sCaptureProbes*>& GetCapturesReferences()
		{
			return g_CaptureProbesReferences;
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
					g_CaptureProbesList[k].EnvironmentProbe.StartCapture(
						[&]{
							GLuint passesPerFace = static_cast<GLuint>((i_renderThreadData->s_renderPasses.size() - shadowmapPassesCount) / 6.f);
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
									_uniformBuffer_frame->Update(&_frame);
									// Execute pass function
									i_renderThreadData->s_renderPasses[_currentRenderPass].RenderPassFunction();
								}

							}
						}
					);

					// After capturing the scene, generate the mip map by opengl itself
					glBindTexture(GL_TEXTURE_CUBE_MAP, g_CaptureProbesList[k].EnvironmentProbe.GetCubemapTextureID());
					glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
					glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
				}

				// iii. start to capture the irradiance map
				{
					glFrontFace(GL_CW);
					_currentEffct = Graphics::GetEffectByKey(EET_IrradConvolution);
					_currentEffct->UseEffect();

					_currentEffct->SetInteger("cubemapTex", 0);
					_currentEffct->ValidateProgram();
					cTexture* _envProbeTexture = cTexture::s_manager.Get(g_CaptureProbesList[k].EnvironmentProbe.GetCubemapTextureHandle());
					_envProbeTexture->UseTexture(GL_TEXTURE0);

					g_CaptureProbesList[k].IrradianceProbe.StartCapture(
						[&]
						{
							for (size_t i = 0; i < 6; ++i)
							{
								glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, g_CaptureProbesList[k].IrradianceProbe.GetCubemapTextureID(), 0);
								glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

								UniformBufferFormats::sFrame _cubemapFrameData(g_CaptureProbesList[k].IrradianceProbe.GetProjectionMat4(), g_CaptureProbesList[k].IrradianceProbe.GetViewMat4(i));
								_uniformBuffer_frame->Update(&_cubemapFrameData);

								// Render cube
								cModel* _cube = cModel::s_manager.Get(Graphics::GetPrimitive(EPT_Cube));
								if (_cube) { _cube->RenderWithoutMaterial(); }
							}
						}
					);

					_currentEffct->UnUseEffect();

				}

				// iv. start to capture the pre-filter cube map
				{
					_currentEffct = Graphics::GetEffectByKey(EET_CubemapPrefilter);
					_currentEffct->UseEffect();

					_currentEffct->SetInteger("cubemapTex", 0);
					_currentEffct->SetInteger("resolution", g_CaptureProbesList[k].EnvironmentProbe.GetWidth());

					_currentEffct->ValidateProgram();
					cTexture* _envProbeTexture = cTexture::s_manager.Get(g_CaptureProbesList[k].EnvironmentProbe.GetCubemapTextureHandle());
					_envProbeTexture->UseTexture(GL_TEXTURE0);

					g_CaptureProbesList[k].PrefilterProbe.StartCapture(
						[&] {
							constexpr GLuint maxMipLevels = 5;
							for (GLuint i = maxMipLevels; i > 0; --i) // capture multiple mip-map levels from low res to high res
							{
								GLuint mip = i - 1;
								// resize frame buffer according to mip-level size.
								GLuint mipWidth = static_cast<GLuint>(g_CaptureProbesList[k].PrefilterProbe.GetWidth() * glm::pow(0.5f, mip));
								GLuint mipHeight = static_cast<GLuint>(g_CaptureProbesList[k].PrefilterProbe.GetHeight() * glm::pow(0.5f, mip));

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
									_uniformBuffer_frame->Update(&_cubemapFrameData);

									// Render cube
									cModel* _cube = cModel::s_manager.Get(Graphics::GetPrimitive(EPT_Cube));
									if (_cube) { _cube->RenderWithoutMaterial(); }
								}
							}
						}
					);

					_currentEffct->UnUseEffect();
					glFrontFace(GL_CCW);
				}
			}
		}

		void UpdatePointOfInterest(const glm::vec3& i_position)
		{
			// clear references
			for (size_t i = 0; i < g_MaximumOverlapVolumes; ++i) g_capturesReadyToBeMixed[i] = nullptr;
			g_capturesReadyToBeMixedCount = 0;

			auto _intersectProbes = g_EnvironmentCaptureAccelerationTree.GetIntersectProbes(i_position);
			const GLuint numOfShape = glm::min(_intersectProbes.size(), static_cast<size_t>(g_MaximumOverlapVolumes));
			UniformBufferFormats::sEnvCaptureWeight captureWeights;

			if (numOfShape > 0)
			{
				if (numOfShape > 1)
				{
					sCaptureProbes* _POI_inInnerBV_captureProbesRef = nullptr; // if POI is in inner BV, this pointer will point that capture probes
					for (auto it : _intersectProbes)
					{
						// Calculate the influence weight
						it->CalcInfluenceWeight(i_position);

						assert(it->Influence >= 0.0f); // Make sure the POI is inside the outer sphere
						// Only record the first InnerBV 
						if (it->Influence > 1.0 ) { 
							if(_POI_inInnerBV_captureProbesRef == nullptr)
								_POI_inInnerBV_captureProbesRef = it; 
							else
							{
								// if POI is closer to the center of the new one, update the inner BV
								if (glm::distance2(it->InnerBV.c(), i_position) < glm::distance2(_POI_inInnerBV_captureProbesRef->InnerBV.c(), i_position))
									_POI_inInnerBV_captureProbesRef = it;
							}
						}
					}
					// if POI is in any inner BV of intersected probes, Use this probes and the remaining references are nullptr
					if (_POI_inInnerBV_captureProbesRef != nullptr)
					{
						captureWeights.Weights[0] = 1.0f;
						g_capturesReadyToBeMixed[0] = _POI_inInnerBV_captureProbesRef;
						g_capturesReadyToBeMixedCount = 1;
					}
					else // Get the mixing weights
					{
						glm::vec4 blendWeights;
						float sumBlendWeight = 0;
						// sort the intersecting probes by most influential, the bigger CaptureProbes->Influence is, the closer to the center, the more important
						std::sort(_intersectProbes.begin(), _intersectProbes.end(), [](const sCaptureProbes* a, const sCaptureProbes* b) { return a->Influence > b->Influence; });
/*
						for (int i = 0; i < _intersectProbes.size(); ++i)
						{
							printf("IW%d:%f ", i, _intersectProbes[i]->Influence);
						}
						printf("_____");*/
						// Calculate sumIW and InvSumIW
						float sumIW = 0;
						float invSumIW = 0;
						for (size_t i = 0; i < numOfShape; ++i) {
							//float _ndf = _intersectProbes[i]->BV.NDF(i_position);
							float _ndf = _intersectProbes[i]->Influence;
							sumIW += _ndf;
							invSumIW += 1.0f - _ndf;
						}

						for (size_t i = 0; i < numOfShape; ++i)
						{
							//float _ndf = _intersectProbes[i]->BV.NDF(i_position);
							float _ndf = _intersectProbes[i]->Influence;
							blendWeights[i] = (_ndf / sumIW) / (numOfShape - 1);
							blendWeights[i] *=  _ndf / invSumIW;
							sumBlendWeight += blendWeights[i];
						}
						assert(sumBlendWeight > 0);
						const float normlizedValue = 1.0f / sumBlendWeight;
						for (size_t i = 0; i < numOfShape; ++i) {
							blendWeights[i] *= normlizedValue;
							captureWeights.Weights[i] = blendWeights[i];
							g_capturesReadyToBeMixed[i] = _intersectProbes[i];
							g_capturesReadyToBeMixedCount = numOfShape;
						}
					}
				}
				else
				{
					captureWeights.Weights[0] = 1.0f;
					g_capturesReadyToBeMixed[0] = _intersectProbes[0];
					g_capturesReadyToBeMixedCount = 1;
				}
			}
			// Update weight data in the GPU
			g_uniformBuffer_EnvCaptureWeight.Update(&captureWeights);
			//printf("w1:%f , w2:%f ,w3:%f ,w4:%f. \n", captureWeights.Weights.x, captureWeights.Weights.y, captureWeights.Weights.z, captureWeights.Weights.w);
		}

		const Graphics::EnvironmentCaptureManager::sCaptureProbes& GetCaptureProbesAt(int i_idx)
		{
			assert(i_idx < g_MaximumOverlapVolumes);

			return *g_capturesReadyToBeMixed[i_idx];
		}

		GLuint GetReadyCapturesCount()
		{
			return g_capturesReadyToBeMixedCount;
		}

		const GLuint MaximumCubemapMixingCount()
		{
			return g_MaximumOverlapVolumes;
		}

	}
}