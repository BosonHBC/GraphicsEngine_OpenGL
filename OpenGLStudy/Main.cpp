#include <stdio.h>
#include <string>
#include <vector>
#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Mesh/Mesh.h"
#include "Effect/Effect.h"
// constants definition
// -----------------------
#define ToRadian(x) x * 0.0174532925f

const GLint WIDTH = 800, HEIGHT = 600;
GLFWwindow* s_mainWindow;

float s_aspect;

std::vector<Mesh*> s_renderList;
std::vector<Effect*> s_effectList;
// Function definition
// ----------------------------
/** InitializeGL GLFW*/
bool InitializeGL();

bool InitGLFW();
bool SetupGLFWWindow(GLFWwindow*& o_mainWindow, const char* windowName, GLFWmonitor* i_monitor = nullptr, GLFWwindow* i_sharedWindow = nullptr);
/** InitializeGL GLEW*/
bool InitGLEW();
/** Create shaders*/
void CreateEffect();
/** Create triangle*/
void CreateTriangle();

int main()
{
	if (!InitializeGL()) return 1;

	// Create triangle
	CreateTriangle();
	//Compile shaders
	CreateEffect();

	glm::mat4 mProjection = glm::perspective(45.0f, (GLfloat)WIDTH / (GLfloat)HEIGHT, 1.0f, 150.0f);

	// loop until window closed
	while (!glfwWindowShouldClose(s_mainWindow))
	{
		// get + handle user input events
		glfwPollEvents();

		// clear window
		glClearColor(0.8f, 0.8f, 0.8f, 1.f);
		// A lot of things can be cleaned like color buffer, depth buffer, so we need to specify what to clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		{
			s_effectList[0]->UseEffect();

			glm::mat4 model = glm::identity<glm::mat4>();
			model = glm::translate(model, glm::vec3(0,0,-2.5f));
			model = glm::rotate(model, ToRadian(45), glm::vec3(0, 0, 1));
			model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));
			glUniformMatrix4fv(s_effectList[0]->GetModelMatrixUniformID(), 1, GL_FALSE, glm::value_ptr(model));
			glUniformMatrix4fv(s_effectList[0]->GetProjectionMatrixUniformID(), 1, GL_FALSE, glm::value_ptr(mProjection));

			s_renderList[0]->Render();
			// clear program
			glUseProgram(0);
		}


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

bool InitializeGL()
{
	// Init GLFW
	if (!InitGLFW()) return false;

	// Create window
	if (!SetupGLFWWindow(s_mainWindow, "MainWindow")) return false;

	// Get buffer size information
	int bufferWidth, bufferHeight;
	glfwGetFramebufferSize(s_mainWindow, &bufferWidth, &bufferHeight);

	s_aspect = (float)bufferWidth / (float)bufferHeight;

	// Set context for GLFW to use, let opengl know that which window is going to be drawn in
	// By calling this function, we can switch between different windows
	glfwMakeContextCurrent(s_mainWindow);

	// InitializeGL GLEW
	if (!InitGLEW()) {
		glfwDestroyWindow(s_mainWindow);
		glfwTerminate();
		return false;
	}

	// Set up viewport, which part of the window will draw things
	glViewport(0, 0, bufferWidth, bufferHeight);

	// enable features
	{
		glEnable(GL_DEPTH_TEST);
	}
	return true;
}

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

// create triangle
void CreateTriangle() {

	GLuint indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1 ,2
	};

	GLfloat vertices[] = {
		-1.0f, -1.0f, 0.0f,
		0.0, -1.0, 1.0f,
		1.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};

	Mesh* triangle = new Mesh();

	triangle->CreateMesh(vertices, indices, 12, 12);
	s_renderList.push_back(triangle);
}


void CreateEffect()
{
	Effect* defaultEffect = new Effect();
	if (!defaultEffect->CreateProgram()) {
		exit(1);
		return;
	}
	s_effectList.push_back(defaultEffect);
}