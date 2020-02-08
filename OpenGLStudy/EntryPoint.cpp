#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>

#endif // DEBUG
#include "Assignments/Assignment.h"

const GLint WIDTH = 800, HEIGHT = 600; // 1280:1024 || 1024:768 || 1366 : 768

int main()
{
	Assignment* _myGame =  new Assignment();

	if (!_myGame->Initialize(WIDTH, HEIGHT, "Assignment")) {
		printf("Failed to initialize application!");
		return 1;
	}
	_myGame->PostInitialization();
	_myGame->Run();
	_myGame->CleanUp();
	safe_delete(_myGame);

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif
	return 0;
}

