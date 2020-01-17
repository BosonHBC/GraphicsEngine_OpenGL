#include "Assignment1.h"
#include "Graphics/Window/Window.h"
#include "assert.h"


bool Assignment1::Initialize(GLuint i_width, GLuint i_height, const char* i_windowName /*= "Default Window"*/)
{
	auto result = true;
	if (!(result = cApplication::Initialize(i_width, i_width, i_windowName))) {
		assert(false, "Failed to initialize Application!");
		return false;
	}

}

void Assignment1::Run()
{
	// loop until window closed
	while (!m_shouldApplicationLoopExit)
	{
		glfwPollEvents();

		// clear window
		glClearColor(0, 0, 0, 1.f);
		// A lot of things can be cleaned like color buffer, depth buffer, so we need to specify what to clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// main draw happens in this scope
		// --------------------------------------------------
		{

		}
		// --------------------------------------------------

		// clear program
		glUseProgram(0);

		// ----------------------
		// Swap buffers
		m_window->SwapBuffers();
	}
}

void Assignment1::CleanUp()
{

}

void Assignment1::UpdateBasedOnTime(float DeltaSeconds)
{

}
