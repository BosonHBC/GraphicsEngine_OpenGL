#pragma once
#include "Application/Application.h"
#include <vector>
#include "Color/Color.h"

class cCamera;
namespace Graphics {
	class cModel;
	class cEffect;
	class cPointLight;
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
	Graphics::cModel* m_teapot;
	std::vector<Graphics::cEffect*> m_effectList;
	cCamera* m_mainCamera;

	Graphics::cPointLight* m_PointLight;
};

