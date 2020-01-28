#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // DEBUG
#include "Assignments/Assignment.h"
#include "MyGame/MyGame.h"

const GLint WIDTH = 800, HEIGHT = 600; // 1280:1024 || 1024:768 || 1366 : 768

int main()
{
	Assignment* _myGame = new Assignment();

	if (!_myGame->Initialize(WIDTH, HEIGHT, "cMyGame")) {
		printf("Failed to initialize application!");
		return 1;
	}
	_myGame->PostInitialization();
	_myGame->Run();
	
	delete _myGame;

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif
	return 0;
}

