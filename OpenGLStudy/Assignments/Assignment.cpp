#include "Assignment.h"
#include "Graphics/Window/Window.h"
#include "assert.h"
#include "GL/glew.h"
#include "Color/Color.h"

Color s_clearColor;

bool Assignment::Initialize(GLuint i_width, GLuint i_height, const char* i_windowName /*= "Default Window"*/)
{
	auto result = true;
	if (!(result = cApplication::Initialize(i_width, i_width, i_windowName))) {
		assert(false, "Failed to initialize Application!");
		return false;
	}

	glfwSwapInterval(1);
	return result;
}

void Assignment::Run()
{
	// loop until window closed
	while (!m_shouldApplicationLoopExit)
	{	
		// clear window
		glClearColor(s_clearColor.r, s_clearColor.g, s_clearColor.b, 1.f);
		glfwPollEvents();
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

void Assignment::CleanUp()
{

}

void Assignment::Tick(float second_since_lastFrame)
{


}

void Assignment::FixedTick()
{

}
