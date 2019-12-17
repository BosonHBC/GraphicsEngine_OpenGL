#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // DEBUG



#include <string>
#include <vector>
#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Window/Window.h"
#include "Mesh/Mesh.h"
#include "Effect/Effect.h"
#include "Camera/Camera.h"
// constants definition
// -----------------------
#define ToRadian(x) x * 0.0174532925f

const GLint WIDTH = 800, HEIGHT = 600;
GLFWwindow* s_mainWindow;

std::vector<Graphics::cMesh*> s_renderList;
std::vector<Graphics::cEffect*> s_effectList;
cCamera* s_mainCamera;
// Function definition
// ----------------------------
void CreateEffect();
void CreateTriangle();
void CreateCamera();

void CreateCamera()
{
	s_mainCamera = new cCamera();
}

int main()
{
	cWindow* newWindow = new cWindow(WIDTH, HEIGHT);
	if (!newWindow->Initialzation()) {
		printf("Failed to initialize openGL window!");
		return 1;
	}
	// Create triangle
	CreateTriangle();
	//Compile shaders
	CreateEffect();
	// create camera
	CreateCamera();

	glm::mat4 mProjection = glm::perspective(45.0f, (GLfloat)WIDTH / (GLfloat)HEIGHT, 1.0f, 150.0f);

	// loop until window closed
	while (!newWindow->GetShouldClose())
	{
		// get + handle user input events
		{
			glfwPollEvents();

			s_mainCamera->CameraControl(newWindow->GetWindowInput());
		}

		// clear window
		glClearColor(0.8f, 0.8f, 0.8f, 1.f);
		// A lot of things can be cleaned like color buffer, depth buffer, so we need to specify what to clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Draw
		for (int i = 0; i < s_renderList.size(); ++i)
		{
			s_effectList[0]->UseEffect();

			glm::mat4 model = glm::identity<glm::mat4>();
			model = glm::translate(model, glm::vec3(0, 0, -2.5f));
			model = glm::rotate(model, ToRadian(45), glm::vec3(0, 0, 1));
			model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));
			glUniformMatrix4fv(s_effectList[0]->GetModelMatrixUniformID(), 1, GL_FALSE, glm::value_ptr(model));
			glUniformMatrix4fv(s_effectList[0]->GetProjectionMatrixUniformID(), 1, GL_FALSE, glm::value_ptr(mProjection));
			glUniformMatrix4fv(s_effectList[0]->GetViewMatrixUniformID() , 1, GL_FALSE, glm::value_ptr(s_mainCamera->GetViewMatrix()));
			s_renderList[0]->Render();
			// clear program
			glUseProgram(0);
		}

		// ----------------------
		// Swap buffers
		newWindow->SwapBuffers();
	}
	// Clean up memories
	{
		delete s_mainCamera;
		delete newWindow;
		newWindow = nullptr;
		for (auto it = s_renderList.begin(); it != s_renderList.end(); ++it)
		{
			delete *it;
			(*it) = nullptr;
		}
		for (auto it = s_effectList.begin(); it != s_effectList.end(); ++it)
		{
			delete *it;
			(*it) = nullptr;
		}
		s_renderList.clear();
		s_effectList.clear();
		s_renderList.~vector();
		s_effectList.~vector();
	}

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif
	return 0;
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

	Graphics::cMesh* triangle = new Graphics::cMesh();

	triangle->CreateMesh(vertices, indices, 12, 12);
	s_renderList.push_back(triangle);
}


void CreateEffect()
{
	Graphics::cEffect* defaultEffect = new Graphics::cEffect();
	if (!defaultEffect->CreateProgram()) {
		exit(1);
		return;
	}
	s_effectList.push_back(defaultEffect);
}