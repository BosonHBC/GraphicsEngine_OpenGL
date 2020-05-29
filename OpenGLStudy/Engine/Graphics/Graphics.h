#pragma once
#include <mutex>

#include "GL/glew.h"
#include "glfw/glfw3.h"
#include "glm/gtc/type_ptr.hpp"

#include "Color/Color.h"
#include "Effect/Effect.h"
#include "RenderStructs.h"
#include "Camera/Camera.h"
#include "UniformBuffer/UniformBuffer.h"
#include "FrameBuffer/cFrameBuffer.h"
#include "Graphics/UniformBuffer/UniformBufferFormats.h"
#include "Graphics/EnvironmentProbes/EnvProbe.h"

// Graphics stores, initializes, cleans up all data that needs to be rendered
namespace Graphics {


	/** Initialization and clean up function*/
	bool Initialize();
	void PreRenderFrame(); // Render frames before rendering the actual scene, like rendering the irradiance cube map, baking static light stuffs
	void RenderFrame();
	bool CleanUp();
	
	void RenderScene(cEffect* const i_effect, GLenum i_drawMode = GL_TRIANGLES);

	void DirectionalShadowMap_Pass();
	void SpotLightShadowMap_Pass();
	void PointLightShadowMap_Pass();

	void BlinnPhong_Pass();
	void PBR_Pass();
	void CubeMap_Pass();

	void EditorPass();
	void Gizmo_RenderVertexNormal();
	void Gizmo_RenderTriangulation();
	void Gizmo_DrawDebugCaptureVolume();
	bool IsTransformGizmoIsHovered(uint32_t i_selectionID, int& o_direction);
	void SetArrowBeingSelected(bool i_selected, int i_direction);

	/** Submit function*/
	void SubmitGraphicSettings(const ERenderMode& i_renderMode);
	void SubmitDataToBeRendered(const UniformBufferFormats::sFrame& i_frameData, const std::vector<std::pair<Graphics::cModel, cTransform>>& i_modelToTransform_map, void(*func_ptr)());
	void SubmitClipPlaneData(const glm::vec4& i_plane0, const glm::vec4& i_plane1 = glm::vec4(0, 0, 0, 0), const glm::vec4& i_plane2 = glm::vec4(0, 0, 0, 0), const glm::vec4& i_plane3 = glm::vec4(0, 0, 0, 0));
	void SubmitPostProcessingData(const UniformBufferFormats::sPostProcessing& i_ppData, float i_ssaoRadius, float i_ssaoPower);
	void SubmitLightingData(const std::vector<cPointLight>& i_pointLights, const std::vector<cSpotLight>& i_spotLights, const cAmbientLight& i_ambientLight, const cDirectionalLight& i_directionalLight);
	void SubmitIOData(const glm::vec2 & i_mousePos, const glm::vec2& i_mousePoseDelta, bool* i_buttonDowns);
	void SubmitSelectionData(uint32_t i_selectionID, const std::vector<std::pair<cModel, cTransform>>& i_modelTransformPairForSelection);

#ifdef ENABLE_CLOTH_SIM
	void SubmitParticleData();
#endif // ENABLE_CLOTH_SIM
	void ClearApplicationThreadData();
	void SetCurrentPass(int i_currentPass);

	/** Usage function*/
	bool CreateEffect(const eEffectType& i_key, const char* i_vertexShaderPath, const char* i_fragmentShaderPath, const char* i_geometryShaderPath = "", const char* const i_TCSPath = "", const char* const i_TESPath = "");
	bool CreateEffect(const eEffectType& i_key, const char* i_computeShaderPath);
	cEffect* GetEffectByKey(const eEffectType& i_key);
	bool RetriveShadowMapIndexAndSubRect(int i_lightIdx, int& io_shadowmapIdx, int& io_resolutionIdx);

	/** Lighting related*/
	UniformBufferFormats::sLighting& GetGlobalLightingData();
	bool CreateAmbientLight(const Color& i_color, cAmbientLight*& o_ambientLight);
	bool CreatePointLight(const glm::vec3& i_initialLocation, const Color& i_color, const GLfloat& i_radius, bool i_enableShadow, cPointLight*& o_pointLight, int uniqueID);
	bool CreateSpotLight(const glm::vec3& i_initialLocation, const glm::vec3& i_direction, const Color& i_color, const GLfloat& i_edge, const GLfloat& i_radius, bool i_enableShadow, cSpotLight*& o_spotLight);
	bool CreateDirectionalLight(const Color& i_color, glm::vec3 i_direction, bool i_enableShadow, cDirectionalLight*& o_directionalLight);
	void UpdateLightingData();
	/** Threading related */
	void Notify_DataHasBeenSubmited();
	// When the data is swapped, application data can be cleared and it is ready for next submission
	void MakeApplicationThreadWaitForSwapingData(std::mutex& i_applicationMutex);
	void MakeApplicationThreadWaitUntilPreRenderFrameDone(std::mutex& i_applicationMutex);

	void SwapDataFromRenderThread();
	/** Others */
	cEnvProbe* GetHDRtoCubemap();
	cUniformBuffer* GetUniformBuffer(const eUniformBufferType& i_uniformBufferType);
	cFrameBuffer* GetOmniShadowMapAt(int i_idx);
	/** Predefined model and textures*/
	const cModel& GetPrimitive(const EPrimitiveType& i_primitiveType);

	glm::vec2 GetTileMinMaxDepthOnCursor(int i_x, int i_y);


	/** Return data related*/
	const sDataReturnToApplicationThread& GetDataFromRenderThread();
}