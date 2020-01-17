#pragma once
#include "Application/Application.h"

class Assignment1 : public Application::cApplication
{
public:
	Assignment1() {};
	~Assignment1()
	{
		CleanUp();
	};

	virtual bool Initialize(GLuint i_width, GLuint i_height, const char* i_windowName = "Default Window");
	virtual void Run();
	virtual void CleanUp();

	void UpdateBasedOnTime(float second_since_lastFrame);

private:


};

