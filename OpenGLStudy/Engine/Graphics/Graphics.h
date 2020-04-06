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
#define SHADOWMAP_START_TEXTURE_UNIT 13

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

	void Gizmo_RenderTransform();
	void Gizmo_RenderVertexNormal();
	void Gizmo_RenderTriangulation();

	/** Submit function*/
	void SubmitGraphicSettings(const ERenderMode& i_renderMode);
	void SubmitDataToBeRendered(const UniformBufferFormats::sFrame& i_frameData, const std::vector<std::pair<Graphics::cModel::HANDLE, cTransform>>& i_modelToTransform_map, void(*func_ptr)());
	void SubmitClipPlaneData(const glm::vec4& i_plane0, const glm::vec4& i_plane1 = glm::vec4(0, 0, 0, 0), const glm::vec4& i_plane2 = glm::vec4(0, 0, 0, 0), const glm::vec4& i_plane3 = glm::vec4(0, 0, 0, 0));
	void SubmitPostProcessingData(const float i_exposure);
	void SubmitLightingData(const std::vector<cPointLight>& i_pointLights, const std::vector<cSpotLight>& i_spotLights, const cAmbientLight& i_ambientLight, const cDirectionalLight& i_directionalLight);
	void SubmitParticleData(const glm::vec3* i_particles);
	void ClearApplicationThreadData();
	void SetCurrentPass(int i_currentPass);

	/** Usage function*/
	bool CreateEffect(const eEffectType& i_key, const char* i_vertexShaderPath, const char* i_fragmentShaderPath, const char* i_geometryShaderPath = "", const char* const i_TCSPath = "", const char* const i_TESPath = "");
	cEffect* GetEffectByKey(const eEffectType& i_key);

	/** Lighting related*/
	UniformBufferFormats::sLighting& GetGlobalLightingData();
	bool CreateAmbientLight(const Color& i_color, cAmbientLight*& o_ambientLight);
	bool CreatePointLight(const glm::vec3& i_initialLocation, const Color& i_color, const GLfloat& i_radius, bool i_enableShadow, cPointLight*& o_pointLight);
	bool CreateSpotLight(const glm::vec3& i_initialLocation, const glm::vec3& i_direction, const Color& i_color, const GLfloat& i_edge, const GLfloat& i_radius, bool i_enableShadow, cSpotLight*& o_spotLight);
	bool CreateDirectionalLight(const Color& i_color, glm::vec3 i_direction, bool i_enableShadow, cDirectionalLight*& o_directionalLight);
	void UpdateLightingData();
	/** Threading related */
	void Notify_DataHasBeenSubmited();
	// When the data is swapped, application data can be cleared and it is ready for next submission
	void MakeApplicationThreadWaitForSwapingData(std::mutex& i_applicationMutex);
	void MakeApplicationThreadWaitUntilPreRenderFrameDone(std::mutex& i_applicationMutex);
	/** Others */
	cEnvProbe* GetHDRtoCubemap();
	cUniformBuffer* GetUniformBuffer(const eUniformBufferType& i_uniformBufferType);

	/** Predefined model and textures*/
	 cModel::HANDLE GetPrimitive(const EPrimitiveType& i_primitiveType);
}