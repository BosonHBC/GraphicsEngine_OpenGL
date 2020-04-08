#pragma once
#include "Application/Application.h"
#include <vector>
#include "Color/Color.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

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
	namespace UniformBufferFormats
	{
		struct sFrame;
	}
}
//----------------------------------------------
class Assignment : public Application::cApplication
{
public:
	bool Initialize(GLuint i_width, GLuint i_height, const char* i_windowName = "Default Window");
	void Run();
	void CleanUp();
	void BeforeUpdate();

	void Tick(float second_since_lastFrame);
	void FixedTick();
private:
	void CreateActor();
	void CreateCamera();
	void CreateLight();

	void SubmitDataToBeRender(const float i_seconds_elapsedSinceLastLoop) override;
	void SubmitLightingData();
	void SubmitSceneData(Graphics::UniformBufferFormats::sFrame* const i_frameData);
	void SubmitSceneDataForEnvironmentCapture(Graphics::UniformBufferFormats::sFrame* const i_frameData);
	void SubmitShadowData();


	Color m_clearColor = Color(0,0,0);
	cEditorCamera* m_editorCamera;

	Graphics::cPointLight* pLight1;
	Graphics::cPointLight* pLight2;
	Graphics::cAmbientLight* aLight;
	Graphics::cDirectionalLight* dLight;
	Graphics::cSpotLight* spLight;
	Graphics::cSpotLight* spLight2;

	cActor* m_teapot;
	cActor* m_teapot2;
	cActor* m_cubemap;
	cActor* m_spaceHolder;

	float m_exposureOffset = 3.0f;
};

