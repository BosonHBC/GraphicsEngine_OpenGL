#include "MyGame.h"

const GLint WIDTH = 800, HEIGHT = 600;

bool cMyGame::Initialize()
{
	auto result = true;
	result = cApplication::Initialize(800, 600);

	return result;
}


void cMyGame::Run()
{

}

void cMyGame::UpdateBasedOnTime(float DeltaSeconds)
{

}


void cMyGame::CleanUp()
{
	cApplication::CleanUp();
}