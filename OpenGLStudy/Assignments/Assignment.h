#pragma once
#include "Application/Application.h"
#include <vector>
#include "Color/Color.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Math/Shape/Sphere.h"
#include "Graphics/UniformBuffer/UniformBufferFormats.h"
#include "Graphics/Model/Model.h"
/** Forward deceleration*/
//----------------------------------------------
class cEditorCamera;
class cActor;
class cTransform;
namespace Graphics {
	class cEffect;
	class cPointLight;
	class cAmbientLight;
	class cDirectionalLight;
	class cSpotLight;
	class cEnvProbe;
}
//----------------------------------------------
class Assignment : public Application::cApplication
{
public:
	static const int s_maxTeapotCount = 8;
	static const int s_maxPLightCount = 80;

	bool Initialize(GLuint i_width, GLuint i_height, const char* i_windowName = "Default Window");
	void Run();
	void CleanUp();
	void BeforeUpdate();

	void Tick(float second_since_lastFrame);
	void FixedTick();
	void EditorGUI() override;
private:
	void CreateActor();
	void CreateCamera();
	void CreateLight();

	void SubmitDataToBeRender(const float i_seconds_elapsedSinceLastLoop) override;
	void SubmitLightingData();
	void SubmitSceneData(std::vector<std::pair<Graphics::cModel, cTransform>>& io_sceneData, Graphics::UniformBufferFormats::sFrame* const i_frameData);
	void SubmitSceneDataForEnvironmentCapture(Graphics::UniformBufferFormats::sFrame* const i_frameData);
	void SubmitShadowData();

	void CreatePointLight(const glm::vec3& i_initialLocation, const Color& i_color, const GLfloat& i_radius, bool i_enableShadow);

	Color m_clearColor = Color(0,0,0);
	cEditorCamera* m_editorCamera = nullptr;

	int m_createdPLightCount = 80;
	Graphics::cPointLight* m_pLights[s_maxPLightCount] = {nullptr};
	cSphere m_collisionSpheres[Assignment::s_maxPLightCount];

	Graphics::cAmbientLight* aLight = nullptr;
	Graphics::cDirectionalLight* dLight = nullptr;
	Graphics::cSpotLight* spLight = nullptr;
	Graphics::cSpotLight* spLight2 = nullptr;
	Color m_pointLightColor = Color::White() * 0.5f;

	int m_renderingTeapotCount = 1;
	cActor* m_teapots[s_maxTeapotCount] = {nullptr};
	cActor* m_cubemap = nullptr;
	cActor* m_spaceHolder = nullptr;
	cActor* m_collisionSphere = nullptr;
	cActor* m_supplyBox = nullptr;

	Graphics::UniformBufferFormats::sPostProcessing m_ppData;
	float m_ssaoRadius = 20;
	float m_ssaoPower = 5;
};

