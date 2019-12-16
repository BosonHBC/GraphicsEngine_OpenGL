#include <stdio.h>
#include "Window.h"


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
		if (!SetupGLFWWindow(m_window, "MainWindow")) return false;
	}

	//02. Set buffer size information
	{
		glfwGetFramebufferSize(m_window, &m_bufferWidth, &m_bufferHeight);
	}

	//03.Set context for GLFW to use, let opengl know that which window is going to be drawn in
	{
		// By calling this function, we can switch between different windows
		glfwMakeContextCurrent(m_window);
	}

	//04. InitializeGL GLEW
	{
		//Allow modern extension features, allow us to use modern extension, which can make things easier
		glewExperimental = GL_TRUE;
		if (glewInit() != GLEW_OK) {
			glfwDestroyWindow(m_window);
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
	return true;
}

void cWindow::CleanUp()
{
	if (m_window) {
		glfwDestroyWindow(m_window);
	}
	glfwTerminate();
}

bool cWindow::GetShouldClose() const
{
	return glfwWindowShouldClose(m_window);
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

void cWindow::SwapBuffers()
{
	glfwSwapBuffers(m_window);
}
