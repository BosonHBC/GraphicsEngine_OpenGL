#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // DEBUG
#include "MyGame/MyGame.h"


const GLint WIDTH = 800, HEIGHT = 600;

int main()
{
	cMyGame* _myGame = new cMyGame();

	if (!_myGame->Initialize(WIDTH, HEIGHT)) {
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

