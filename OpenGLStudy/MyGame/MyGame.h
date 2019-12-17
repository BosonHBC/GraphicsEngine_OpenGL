#pragma once
#include "Application/Application.h"

class cMyGame: public Application::cApplication
{
public:
	cMyGame() {};
	~cMyGame() {};

	virtual bool Initialize();
	virtual void Run();
	virtual void CleanUp();

	virtual void UpdateBasedOnTime(float DeltaSeconds);

private:

};
