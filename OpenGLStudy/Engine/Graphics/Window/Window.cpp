#include <stdio.h>
#include "Window.h"
#include "WindowInput.h"

cWindow::~cWindow()
{
	CleanUp();
}

bool cWindow::Initialzation()
{
	//00. Init GLFW
	{
		if (!glfwInit()) {
			printf("GLFW initialization failed!");
			glfwTerminate();
			return false;
		}
	}

	//01. Create window
	{
		if (!SetupGLFWWindow(m_glfwWindow, m_windowName)) return false;
	}

	//02. Set buffer size information
	{
		glfwGetFramebufferSize(m_glfwWindow, &m_bufferWidth, &m_bufferHeight);
	}

	//03.Set context for GLFW to use, let opengl know that which window is going to be drawn in
	{
		// By calling this function, we can switch between different windows
		glfwMakeContextCurrent(m_glfwWindow);
	}

	//04. InitializeGL GLEW
	{
		//Allow modern extension features, allow us to use modern extension, which can make things easier
		glewExperimental = GL_TRUE;
		if (glewInit() != GLEW_OK) {
			glfwDestroyWindow(m_glfwWindow);
			glfwTerminate();
			printf("GLEW Initialization failed!");
			return false;
		}
	}

	//05. Set up viewport, which part of the window will draw things
	{
		glViewport(0, 0, m_bufferWidth, m_bufferHeight);
	}

	//06. enable features
	{
		glEnable(GL_DEPTH_TEST);
	}

	//07. setup input callback
	{
		// Assign user pointer to this window, so that GLFW window knows this cWindow reference so that we can get keys variable
		glfwSetWindowUserPointer(m_glfwWindow, this);
		CreateCallbacks();
		// Set input mode
		glfwSetInputMode(m_glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	return true;
}

void cWindow::CleanUp()
{
	if (m_windowInput) {
		delete m_windowInput;
		m_windowInput = nullptr;
	}
	if (m_glfwWindow) {
		glfwDestroyWindow(m_glfwWindow);
	}
	glfwTerminate();
}

bool cWindow::GetShouldClose() const
{
	return glfwWindowShouldClose(m_glfwWindow);
}

bool cWindow::SetupGLFWWindow(GLFWwindow*& o_mainWindow, const char* i_windowName, GLFWmonitor* i_monitor /*= nullptr*/, GLFWwindow* i_sharedWindow /*= nullptr*/)
{
	// Set the major version of this window to 3, since we are using 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Core profile = No backwards compatibility
	// This line will throw an error when we are trying to use old features, function which might be removed in the future
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// Allow forward compatibility
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	o_mainWindow = glfwCreateWindow(m_bufferWidth, m_bufferHeight, i_windowName, i_monitor, i_sharedWindow);
	if (!o_mainWindow) {
		printf("Create GLFW window failed!");
		glfwTerminate();
		return false;
	}
	return true;
}

void cWindow::CreateCallbacks()
{
	m_windowInput = new sWindowInput();
	glfwSetKeyCallback(m_glfwWindow, &cWindow::HandleKeys);
	glfwSetCursorPosCallback(m_glfwWindow, &cWindow::HandleMouse);
}

void cWindow::HandleKeys(GLFWwindow* i_window, int i_key, int i_code, int i_action, int i_mode)
{
	// Get reference to the instanced window
	cWindow* thisWindow = static_cast<cWindow*>(glfwGetWindowUserPointer(i_window));
	sWindowInput* _input = thisWindow->m_windowInput;

	if (i_key == GLFW_KEY_ESCAPE && i_action == GLFW_PRESS) {
		// tell glfw window that it is time to close
		glfwSetWindowShouldClose(i_window, GL_TRUE);
	}
	if (i_key >= 0 && i_key <= MAX_KEY_LENGTH) {
		const uint64_t keyMask = 1;

		if (i_action == GLFW_PRESS) {
			_input->keyDowns = ((keyMask << i_key) | _input->keyDowns);
		}
		else if (i_action == GLFW_RELEASE) {
			_input->keyDowns = ((~(keyMask << i_key)) & _input->keyDowns);
		}
	}
}

void cWindow::HandleMouse(GLFWwindow* i_window, double i_xPos, double i_yPos)
{
	// Get reference to the instanced window
	cWindow* thisWindow = static_cast<cWindow*>(glfwGetWindowUserPointer(i_window));
	sWindowInput* _input = thisWindow->m_windowInput;

	
	if (_input->isFirstMove) {
		_input->lastX = i_xPos;
		_input->lastY = i_yPos;
		_input->isFirstMove = false;
	}

	_input->dx = i_xPos - _input->lastX;
	// prevent inverted control in Y axis
	_input->dy = _input->lastY - i_yPos;

	_input->lastX = i_xPos;
	_input->lastY = i_yPos;
}

void cWindow::SwapBuffers()
{
	glfwSwapBuffers(m_glfwWindow);
}
