#pragma once
#include <mutex>

#include "GL/glew.h"
#include "glfw/glfw3.h"
#include "glm/gtc/type_ptr.hpp"

#include "Color/Color.h"
#include "Effect/Effect.h"
#include "Light/PointLight/PointLight.h"
#include "Light/AmbientLight/AmbientLight.h"
#include "Light/DirectionalLight/DirectionalLight.h"
#include "Light/SpotLight/SpotLight.h"
#include "Camera/Camera.h"
#include "Model/Model.h"
#include "UniformBuffer/UniformBuffer.h"
#include "FrameBuffer/cFrameBuffer.h"
#include "Graphics/UniformBuffer/UniformBufferFormats.h"
#include "Graphics/EnvironmentProbes/EnvProbe.h"

// Graphics stores, initializes, cleans up all data that needs to be rendered
namespace Graphics {

	/** Initialization and clean up function*/
	bool Initialize();
	void PreRenderFrame(); // Render frames before rendering the actual scene, like rendering the irrandance cube map, baking static light stuffs
	void RenderFrame();
	bool CleanUp();

	void DirectionalShadowMap_Pass();
	void SpotLightShadowMap_Pass();
	void PointLightShadowMap_Pass();
	void Reflection_Pass();

	void Render_Pass();
	void PBR_Pass();
	void CubeMap_Pass();
	void Gizmo_RenderTransform();
	void Gizmo_RenderVertexNormal();

	/** Submit function*/
	void SubmitClipPlaneData(const glm::vec4& i_plane0, const glm::vec4& i_plane1 = glm::vec4(0, 0, 0, 0), const glm::vec4& i_plane2 = glm::vec4(0, 0, 0, 0), const glm::vec4& i_plane3 = glm::vec4(0, 0, 0, 0));
	void SubmitLightingData(const std::vector<cPointLight>& i_pointLights, const std::vector<cSpotLight>& i_spotLights, const cAmbientLight& i_ambientLight, const cDirectionalLight& i_directionalLight);
	void SubmitDataToBeRendered(const UniformBufferFormats::sFrame& i_frameData, const std::vector<std::pair<Graphics::cModel::HANDLE, cTransform>>& i_modelToTransform_map, void(*func_ptr)());

	void ClearApplicationThreadData();
	
	/** Usage function*/
	bool CreateEffect(const char* i_key, const char* i_vertexShaderPath, const char* i_fragmentShaderPath, const char* i_geometryShaderPath = "");
	cEffect* GetEffectByKey(const char* i_key);
	cEffect* GetCurrentEffect();

	/** Lighting related*/
	UniformBufferFormats::sLighting& GetGlobalLightingData();
	bool CreateAmbientLight(const Color& i_color, cAmbientLight*& o_ambientLight);
	bool CreatePointLight(const glm::vec3& i_initialLocation,const Color& i_color,const GLfloat& i_const, const GLfloat& i_linear, const GLfloat& i_quadratic, bool i_enableShadow, cPointLight*& o_pointLight);
	bool CreateSpotLight(const glm::vec3& i_initialLocation, const glm::vec3& i_direction, const Color& i_color, const GLfloat& i_edge, const GLfloat& i_const, const GLfloat& i_linear, const GLfloat& i_quadratic, bool i_enableShadow, cSpotLight*& o_spotLight);
	bool CreateDirectionalLight(const Color& i_color, glm::vec3 i_direction, bool i_enableShadow, cDirectionalLight*& o_directionalLight);
	void UpdateLightingData();
	/** Threading related */
	void Notify_DataHasBeenSubmited();
	// When the data is swapped, application data can be cleared and it is ready for next submission
	void MakeApplicationThreadWaitForSwapingData(std::mutex& i_applicationMutex);

	/** Others */
	cFrameBuffer* GetCameraCaptureFrameBuffer();
	cEnvProbe* GetEnvironmentProbe();
	cEnvProbe* GetIrrdianceMapProbe();
	cEnvProbe* GetPreFilterMapProbe();
	cFrameBuffer* GetBRDFLutFrameBuffer();
}