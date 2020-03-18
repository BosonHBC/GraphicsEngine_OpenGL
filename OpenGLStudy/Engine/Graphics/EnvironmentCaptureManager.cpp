#pragma once
#include "Graphics/EnvironmentCaptureManager.h"
#include "UniformBuffer/UniformBuffer.h"
#include "RenderStructs.h"
#include <stdio.h>
#include <vector>
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
			
			assert(i_environmentCubemapSize > 512);

			if (!(result = g_CaptureProbesList.end()->EnvironmentProbe.Initialize(i_radius, i_environmentCubemapSize, i_environmentCubemapSize, ETT_FRAMEBUFFER_HDR_CUBEMAP, i_position)))
			{
				printf("Fail to create environment probe.\n");
				return result;
			}
			if (!(result = g_CaptureProbesList.end()->IrradianceProbe.Initialize(i_radius, g_IrradianceMapResolution, g_IrradianceMapResolution, ETT_FRAMEBUFFER_HDR_CUBEMAP, i_position)))
			{
				printf("Fail to create Irradiance probe.\n");
				return result;
			}
			if(!(result = g_CaptureProbesList.end()->PrefilterProbe.Initialize(i_radius, g_PrefilterMapResolution, g_PrefilterMapResolution, ETT_FRAMEBUFFER_HDR_MIPMAP_CUBEMAP, i_position)))
			{
				printf("Fail to create Pre-filter probe.\n");
				return result;
			}
		}

		void CaptureEnvironment(Graphics::sDataRequiredToRenderAFrame* i_renderThreadData)
		{

		}

		void UpdatePointOfInterest(const glm::vec3& i_position)
		{

		}

	}
}