#pragma once
#include "Graphics/UniformBuffer/UniformBufferFormats.h"
#include "Light/PointLight/PointLight.h"
#include "Light/AmbientLight/AmbientLight.h"
#include "Light/DirectionalLight/DirectionalLight.h"
#include "Light/SpotLight/SpotLight.h"
#include "Model/Model.h"
#include "vector"
#include "Assignments/ClothSimulation/SimulationParams.h"
namespace Graphics {
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
	// IO struct
	// ------------------------------------------------------------------------------------------------------------------------------------
	struct sIO
	{
		glm::vec2 MousePos = glm::vec2(0.0f); // current mouse position
		glm::vec2 dMousePos = glm::vec2(0.0f); // mouse position delta
		uint8_t g_mouseDown = 0;						// no button is down

		// i_button: (0=left, 1=right, 2=middle)
		void SetButton(bool i_down, int i_button)
		{
			if (i_down)
				g_mouseDown |= (1 << i_button);
			else
				g_mouseDown &= ~(1 << i_button);
		}
		bool IsButtonDown(int i_button)
		{
			return (g_mouseDown >> i_button) & 1;
		}
	};
	// Data required to render a frame, right now do not support switching effect(shader)
	struct sDataRequiredToRenderAFrame
	{
		ERenderMode g_renderMode = ERM_ForwardShading;
		UniformBufferFormats::sClipPlane s_ClipPlane;
		UniformBufferFormats::sPostProcessing s_PostProcessing;
		UniformBufferFormats::sSSAO g_ssaoData;
		std::vector<sPass> g_renderPasses;
		std::vector<std::pair<cModel, cTransform>> g_modelTransformPairForSelectionPass;
		// Lighting data
		std::vector<cPointLight> g_pointLights;
		std::vector<cSpotLight> g_spotLights;
		cAmbientLight g_ambientLight;
		cDirectionalLight g_directionalLight;
		sIO g_IO;
		uint32_t g_selectingItemID;
#ifdef ENABLE_CLOTH_SIM
		// for simulation specifically
		glm::vec3 particles[ClothSim::VC];
		float clothVertexData[ClothSim::VC * 14];
#endif // ENABLE_CLOTH_SIM
	};

	struct sDataReturnToApplicationThread
	{
		uint32_t g_selectionID;
		GLfloat g_deltaRenderAFrameTime;
		GLfloat g_deltaGeometryTime;
		GLfloat g_deltaDeferredLightingTime;
		GLfloat g_deltaPointLightShadowMapTime;
		GLfloat g_deltaSelectionTime;
		GLuint g_tilesIntersectByLight;
	};

	// Primitive types
	enum EPrimitiveType : uint8_t
	{
		EPT_Cube = 0,
		EPT_Arrow = 1,
		EPT_Quad = 2
	};


}