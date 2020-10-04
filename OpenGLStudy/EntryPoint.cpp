#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>

#endif // DEBUG
#include "Assignments/Assignment.h"
#include "Application/Application.h"

const GLint WIDTH = 1280, HEIGHT = 720; // 1280:1024 || 1024:768 || 1366 : 768

int main()
{
	Assignment* _myGame = nullptr;
	auto result = true;

	if (!(result = Application::CreateApplication<Assignment>(_myGame, WIDTH, HEIGHT, "Assignment"))) {
		printf("Failed to create application, main()!\n");
		return 1;
	}

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	if (!(result = _myGame->PostInitialization())) {
		printf("Failed to post initialize the application!\n");
		return 1;
	}

	_myGame->Run();

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	Application::DestroyApplication(_myGame);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
#ifdef _DEBUG
	//_CrtDumpMemoryLeaks();
#endif
	return 0;
}

