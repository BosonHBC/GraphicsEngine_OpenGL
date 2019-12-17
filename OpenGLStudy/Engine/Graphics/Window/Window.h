#pragma once

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#define MAX_KEY_LENGTH 128

// Forward declaration
// ------------------------
struct sWindowInput;
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

	/** Usage function*/
	void UpdateBasedOnTime(float i_elapsedSecondCount_sinceLastUpdate);
	void SwapBuffers();

	/** Getters */
	const GLint& GetBufferWidth() const { return m_bufferWidth; }
	const GLint& GetBufferHeight() const { return m_bufferHeight; }
	bool GetShouldClose() const;
	GLFWwindow* GetWindow() const { return m_window; }
	sWindowInput* const GetWindowInput() const { return m_windowInput; }

private:
	/** private variables*/
	GLFWwindow* m_window;
	GLint m_bufferWidth, m_bufferHeight;
	sWindowInput* m_windowInput;

	/** private helper functions*/
	bool SetupGLFWWindow(GLFWwindow*& o_mainWindow, const char* i_windowName, GLFWmonitor* i_monitor = nullptr, GLFWwindow* i_sharedWindow = nullptr);
	virtual void CreateCallbacks();

	/** static private functions*/
	static void HandleKeys(GLFWwindow* i_window, int i_key, int i_code, int i_action, int i_mode);
	static void HandleMouse(GLFWwindow* i_window, double i_xPos, double i_yPos);
};
