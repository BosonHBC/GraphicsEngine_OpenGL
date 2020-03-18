#pragma once
#include "Graphics/UniformBuffer/UniformBufferFormats.h"
#include "Light/PointLight/PointLight.h"
#include "Light/AmbientLight/AmbientLight.h"
#include "Light/DirectionalLight/DirectionalLight.h"
#include "Light/SpotLight/SpotLight.h"
#include "Model/Model.h"
#include "vector"
namespace Graphics{
	// Rendering thread data and data structures
	// ------------------------------------------------------------------------------------------------------------------------------------
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
}