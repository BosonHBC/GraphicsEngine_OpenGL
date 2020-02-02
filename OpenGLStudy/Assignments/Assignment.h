#pragma once
#include "Application/Application.h"
#include <vector>
#include "Color/Color.h"

class cEditorCamera;
namespace Graphics {
	class cModel;
	class cEffect;
	class cMaterial;
	class cPointLight;
	class cAmbientLight;
}

class Assignment : public Application::cApplication
{
public:
	Assignment() {};
	~Assignment()
	{
		CleanUp();
	};

	virtual bool Initialize(GLuint i_width, GLuint i_height, const char* i_windowName = "Default Window");
	virtual void Run();
	virtual void CleanUp();

	void Tick(float second_since_lastFrame);
	void FixedTick();
private:

	void CreateEffect();
	void CreateCamera();

	Color m_clearColor;
	cEditorCamera* m_editorCamera;
	std::vector<Graphics::cEffect*> m_effectList;
	Graphics::cModel* m_teapot;
	Graphics::cPointLight* m_PointLight;
	Graphics::cAmbientLight* m_ambientLight;
	Graphics::cMaterial* m_material;
};

