#pragma once
#include "Application/Application.h"

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


};

