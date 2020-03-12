#pragma once
#include "Application/Application.h"
#include <vector>
#include "Color/Color.h"

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
}
//----------------------------------------------
class Assignment : public Application::cApplication
{
public:
	virtual bool Initialize(GLuint i_width, GLuint i_height, const char* i_windowName = "Default Window");
	virtual void Run();
	virtual void CleanUp();

	void Tick(float second_since_lastFrame);
	void FixedTick();
private:
	void CreateActor();
	void CreateCamera();
	void CreateLight();

	Color m_clearColor;
	cEditorCamera* m_editorCamera;

	Graphics::cPointLight* pLight1;
	Graphics::cPointLight* pLight2;
	Graphics::cAmbientLight* aLight;
	Graphics::cDirectionalLight* dLight;
	Graphics::cSpotLight* spLight;
	Graphics::cSpotLight* spLight2;

	cActor* m_teapot;
	cActor* m_teapot2;
	cActor* m_mirror;
	cActor* m_cubemap;
	cActor* m_spaceHolder;

	std::vector<cActor*> m_sphereList;
};

