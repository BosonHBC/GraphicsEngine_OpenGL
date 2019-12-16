#pragma once

#include "GL/glew.h"
#include "GLFW/glfw3.h"

// Forward declaration
// ------------------------
struct GLFWwindow;
struct GLFWMonitor;
// OpenGL Window for initializing GLFW, GLEW and create view port
class cWindow
{


public:
	/** Constructors and destructor */
	cWindow() : m_window(nullptr), m_bufferWidth(0), m_bufferHeight(0)
	{}
	cWindow(GLint i_windowWidth, GLint i_windowHeight) : m_window(nullptr),
		m_bufferWidth(i_windowWidth), m_bufferHeight(i_windowHeight)
	{}
	~cWindow();

	/** Initializations and clean up*/
	bool Initialzation();
	void CleanUp();
	void SwapBuffers();

	/** Getters */
	const GLint& GetBufferWidth() const { return m_bufferWidth; }
	const GLint& GetBufferHeight() const { return m_bufferHeight; }
	GLFWwindow* GetWindow() const { return m_window; }
	bool GetShouldClose() const;
private:

	GLFWwindow* m_window;
	GLint m_bufferWidth, m_bufferHeight;

	bool SetupGLFWWindow(GLFWwindow*& o_mainWindow, const char* i_windowName, GLFWmonitor* i_monitor = nullptr, GLFWwindow* i_sharedWindow = nullptr);

};
