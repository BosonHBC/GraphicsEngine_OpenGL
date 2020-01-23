#include "Assignment1.h"
#include "Graphics/Window/Window.h"
#include "assert.h"
#include "GL/glew.h"
#include "Color/Color.h"

Color s_clearColor;
bool s_channelDir[3] = { false };

bool Assignment1::Initialize(GLuint i_width, GLuint i_height, const char* i_windowName /*= "Default Window"*/)
{
	auto result = true;
	if (!(result = cApplication::Initialize(i_width, i_width, i_windowName))) {
		assert(false, "Failed to initialize Application!");
		return false;
	}

	glfwSwapInterval(1);
}

void Assignment1::Run()
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

void Assignment1::CleanUp()
{

}

void Assignment1::Tick(float second_since_lastFrame)
{

	// change r channel
	if (s_clearColor.r >= 1 || s_clearColor.r <= 0) {
		s_channelDir[0] = !s_channelDir[0];
	}
	s_clearColor.r = s_clearColor.r + (s_channelDir[0] ? 1.f : -1.f) * second_since_lastFrame;
	// change g channel
	if (s_clearColor.g >= 1 || s_clearColor.g <= 0) {
		s_channelDir[1] = !s_channelDir[1];
	}
	s_clearColor.g = s_clearColor.g + (s_channelDir[0] ? 1.f : -1.f) *2* second_since_lastFrame;
	// change b channel
	if (s_clearColor.b >=1 || s_clearColor.b <= 0){
		s_channelDir[2] = !s_channelDir[2];
	}
	s_clearColor.b = s_clearColor.b + (s_channelDir[0] ? 1.f : -1.f) *3* second_since_lastFrame;

//	printf("r: %f, g: %f, b: %f \n", s_clearColor.r, s_clearColor.g, s_clearColor.b);

}

void Assignment1::FixedTick()
{

}
