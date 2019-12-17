#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // DEBUG
#include "MyGame/MyGame.h"

const GLint WIDTH = 800, HEIGHT = 600;

int main()
{
	cMyGame* app = new cMyGame();

	if (!app->Initialize(WIDTH, HEIGHT)) {
		printf("Failed to initialize application!");
		return 1;
	}

	app->Run();

	delete app;

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif
	return 0;
}

