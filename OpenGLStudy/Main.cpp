#include <stdio.h>
#include <string>
#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

// constants definition
// -----------------------
const GLint WIDTH = 800, HEIGHT = 600;
GLFWwindow* s_mainWindow;
extern GLuint s_vaoID;
extern GLuint s_vboID;
extern GLuint s_programID;

// Function definition
// ----------------------------
/** Initialize GLFW*/
bool InitGLFW();
bool SetupGLFWWindow(GLFWwindow*& o_mainWindow, const char* windowName, GLFWmonitor* i_monitor = nullptr, GLFWwindow* i_sharedWindow = nullptr);
/** Initialize GLEW*/
bool InitGLEW();
/** Create shaders*/
void CompileShaders();
/** Create triangle*/
void CreateTriangle();
void ApplyTransfomration();
void DrawTriangle();

int main()
{
	// Init GLFW
	if (!InitGLFW()) return 1;

	// Create window
	if (!SetupGLFWWindow(s_mainWindow, "MainWindow")) return 1;

	// Get buffer size information
	int bufferWidth, bufferHeight;
	glfwGetFramebufferSize(s_mainWindow, &bufferWidth, &bufferHeight);

	// Set context for GLFW to use, let opengl know that which window is going to be drawn in
	// By calling this function, we can switch between different windows
	glfwMakeContextCurrent(s_mainWindow);

	// Initialize GLEW
	if (!InitGLEW()) {
		glfwDestroyWindow(s_mainWindow);
		glfwTerminate();
		return 1;
	}

	// Set up viewport, which part of the window will draw things
	glViewport(0, 0, bufferWidth, bufferHeight);

	// enable features
	{
		glEnable(GL_DEPTH_TEST);
	}

	// Create triangle
	CreateTriangle();
	//Comiple shaders
	CompileShaders();

	// loop until window closed
	while (!glfwWindowShouldClose(s_mainWindow))
	{
		// get + handle user input events
		glfwPollEvents();

		// clear window
		glClearColor(0.8f, 0.8f, 0.8f, 1.f);
		// A lot of things can be cleaned like color buffer, depth buffer, so we need to specify what to clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		DrawTriangle();

		// ----------------------
		// Swap buffers
		glfwSwapBuffers(s_mainWindow);
	}

	glfwDestroyWindow(s_mainWindow);
	glfwTerminate();

	return 0;
}


// Function implementation 
// ----------------------------
bool InitGLFW()
{
	if (!glfwInit()) {
		printf("GLFW initialization failed!");
		glfwTerminate();
		return false;
	}
	return true;
}

bool SetupGLFWWindow(GLFWwindow*& o_mainWindow, const char* i_windowName, GLFWmonitor* i_monitor, GLFWwindow* i_sharedWindow)
{
	// Set the major version of this window to 3, since we are using 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Core profile = No backwards compatibility
	// This line will throw an error when we are trying to use old features, function which might be removed in the future
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// Allow forward compatibility
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	o_mainWindow = glfwCreateWindow(WIDTH, HEIGHT, i_windowName, i_monitor, i_sharedWindow);
	if (!o_mainWindow) {
		printf("Create GLFW window failed!");
		glfwTerminate();
		return false;
	}
	return true;
}


bool InitGLEW()
{
	//Allow modern extension features, allow us to use modern extension, which can make things easier
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		printf("GLEW Initialization failed!");
		return false;
	}
	return true;
}
