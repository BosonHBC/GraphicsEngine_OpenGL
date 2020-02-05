#pragma once
#include "Application/Application.h"
#include <vector>
#include "Color/Color.h"
/** Forward deceleration*/
//----------------------------------------------
class cEditorCamera;
class cActor;
namespace Graphics {
	class cEffect;
	class cPointLight;
	class cAmbientLight;
}
//----------------------------------------------
class Assignment : public Application::cApplication
{
public:
	Assignment() {};
	virtual ~Assignment()
	{
	};

	virtual bool Initialize(GLuint i_width, GLuint i_height, const char* i_windowName = "Default Window");
	virtual void Run();
	virtual void CleanUp();

	void Tick(float second_since_lastFrame);
	void FixedTick();
private:

	void CreateEffect();
	void CreateActor();
	void CreateCamera();
	void CreateLight();

	Color m_clearColor;
	cEditorCamera* m_editorCamera;
	std::vector<Graphics::cEffect*> m_effectList;
	Graphics::cEffect* m_currentEffect;
	Graphics::cPointLight* m_PointLight;
	Graphics::cAmbientLight* m_ambientLight;

	cActor* m_teapot;
	cActor* m_teapot2;
};

