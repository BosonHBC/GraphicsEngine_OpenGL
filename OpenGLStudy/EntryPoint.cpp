#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>

#endif // DEBUG
#include "Assignments/Assignment.h"
#include "Application/Application.h"

const GLint WIDTH = 800, HEIGHT = 600; // 1280:1024 || 1024:768 || 1366 : 768

int main()
{
	Assignment* _myGame = nullptr;
	auto result = true;

	if (!(result = Application::CreateApplication<Assignment>(_myGame, WIDTH, HEIGHT, "Assignment"))) {
		printf("Failed to create application!");
		return 1;
	}
	if (!(result = _myGame->PostInitialization())) {
		printf("Failed to post initialize the application!");
		return 1;
	}

	_myGame->Run();

	Application::DestroyApplication(_myGame);

#ifdef _DEBUG
	//_CrtDumpMemoryLeaks();
#endif
	return 0;
}

