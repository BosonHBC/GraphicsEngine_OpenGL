#pragma once
#include "Graphics/UniformBuffer/UniformBufferFormats.h"
#include "Light/PointLight/PointLight.h"
#include "Light/AmbientLight/AmbientLight.h"
#include "Light/DirectionalLight/DirectionalLight.h"
#include "Light/SpotLight/SpotLight.h"
#include "Model/Model.h"
#include "vector"
#include "Assignments/ClothSimulation/SimulationParams.h"
namespace Graphics{
	// render mode
	enum ERenderMode : uint8_t
	{
		ERM_ForwardShading,
		ERM_DeferredShading,
		ERM_Deferred_Albede,
		ERM_Deferred_Metallic,
		ERM_Deferred_Roughness,
		ERM_Deferred_Normal,
		ERM_Deferred_IOR,
		ERM_Deferred_Depth,
		ERM_Deferred_WorldPos,
		ERM_SSAO,
	};
	extern const std::map<uint8_t, const char*> g_renderModeNameMap;

	// Rendering thread data and data structures
	// ------------------------------------------------------------------------------------------------------------------------------------
	struct sPass
	{
		UniformBufferFormats::sFrame FrameData;
		std::vector<std::pair<Graphics::cModel, cTransform>> ModelToTransform_map;
		void(*RenderPassFunction) ();
		sPass() {}
	};
	// Data required to render a frame, right now do not support switching effect(shader)
	struct sDataRequiredToRenderAFrame
	{
		ERenderMode g_renderMode = ERM_ForwardShading;
		UniformBufferFormats::sClipPlane s_ClipPlane;
		UniformBufferFormats::sPostProcessing s_PostProcessing;
		std::vector<sPass> g_renderPasses;
		// Lighting data
		std::vector<cPointLight> g_pointLights;
		std::vector<cSpotLight> g_spotLights;
		cAmbientLight g_ambientLight;
		cDirectionalLight g_directionalLight;

#ifdef ENABLE_CLOTH_SIM
		// for simulation specifically
		glm::vec3 particles[ClothSim::VC];
		float clothVertexData[ClothSim::VC * 14];
#endif // ENABLE_CLOTH_SIM
	};

	struct sDataReturnToApplicationThread
	{
		uint32_t g_selectionID;
	};

	// Primitive types
	enum EPrimitiveType : uint8_t
	{
		EPT_Cube = 0,
		EPT_Arrow = 1,
		EPT_Quad = 2
	};


}