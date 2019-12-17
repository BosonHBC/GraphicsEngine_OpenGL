#pragma once
#include "Application/Application.h"

class cMyGame: public Application::cApplication
{
public:
	cMyGame() {};
	~cMyGame() 
	{
		CleanUp();
	};

	virtual bool Initialize(GLuint i_width, GLuint i_height);
	void Run() ;
	virtual void CleanUp();

	virtual void UpdateBasedOnTime(float DeltaSeconds);

private:

};
