#pragma once

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "Math/Shape/Rect.h"

#define GLFW_MAX_KEY_COUNT 360
#define GLFW_MAX_BUTTON_COUNT 8
// Forward declaration
// ------------------------
struct sWindowInput;
// OpenGL Window for initializing GLFW, GLEW and create view port
class cWindow
{

public:
	/** Constructors and destructor */
	cWindow() : m_glfwWindow(nullptr), m_bufferWidth(0), m_bufferHeight(0)
	{}
	cWindow(GLint i_windowWidth, GLint i_windowHeight, const char* i_windowName) : m_glfwWindow(nullptr),
		m_bufferWidth(i_windowWidth), m_bufferHeight(i_windowHeight), m_windowName(i_windowName)
	{}
	~cWindow();

	/** Initializations and clean up*/
	bool Initialzation();
	void CleanUp();

	/** Usage function*/
	void SwapBuffers();
	// Update the viewport size
	void SetViewportSize(GLuint i_newWidth, GLuint i_newHeight);
	void SetViewPort(const sRect& i_newViewPort);

	/** Getters */
	const GLint& GetBufferWidth() const { return m_bufferWidth; }
	const GLint& GetBufferHeight() const { return m_bufferHeight; }
	GLfloat GetAOR() const { return static_cast<GLfloat>(m_bufferWidth) / m_bufferHeight; }
	bool GetShouldClose() const;
	GLFWwindow* GetWindow() const { return m_glfwWindow; }
	sWindowInput* const GetWindowInput() const { return m_windowInput; }

private:
	/** private variables*/
	GLFWwindow* m_glfwWindow;
	GLint m_bufferWidth, m_bufferHeight;
	const char* m_windowName;
	sWindowInput* m_windowInput;
	bool m_windowMaximumed = false;

	/** private helper functions*/
	bool SetupGLFWWindow(GLFWwindow*& o_mainWindow, const char* i_windowName, GLFWmonitor* i_monitor = nullptr, GLFWwindow* i_sharedWindow = nullptr);
	bool SetupImGUI();
	virtual void CreateCallbacks();

	/** static private functions*/
	static void HandleKeys(GLFWwindow* i_window, int i_key, int i_code, int i_action, int i_mode);
	static void HandleMouse(GLFWwindow* i_window, double i_xPos, double i_yPos);
	static void HandleMouseButton(GLFWwindow* i_window, int i_button, int i_action, int i_mods);
	static void OnWindowSizeChanged(GLFWwindow* window, int width, int height);
};
