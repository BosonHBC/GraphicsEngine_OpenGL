#include "MyGame.h"

#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Graphics/Window/WindowInput.h"
#include "Graphics/Window/Window.h"
#include "Graphics/Mesh/Mesh.h"
#include "Graphics/Texture/Texture.h"
#include "Graphics/Effect/Effect.h"
#include "Graphics/Camera/Camera.h"
// constants definition
// -----------------------
#define ToRadian(x) x * 0.0174532925f

GLFWwindow* s_mainWindow;

std::vector<Graphics::cMesh*> s_renderList;
std::vector<Graphics::cEffect*> s_effectList;
cCamera* s_mainCamera;
cMyGame* s_myGameInstance;

Graphics::cTexture s_brickTexture;
Graphics::cTexture s_woodTexture;


// Function definition
// ----------------------------
void CreateEffect();
void CreateTriangle();
void CreateCamera();
void SetUpTextures();

void CreateCamera()
{
	s_mainCamera = new cCamera();
	float _aspect = (float)s_myGameInstance->Get_GLFW_Window()->GetBufferWidth() / (float)s_myGameInstance->Get_GLFW_Window()->GetBufferHeight();
	s_mainCamera->CreateProjectionMatrix(45.0f, _aspect, 0.1f, 150.0f);
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
		//     x,		y,		z,			u,		v
			-1.0f, -1.0f, 0.0f,		0.0f, 0.0f,
			0.0f, -1.0f, 1.0f,		0.5f, 0.0f,
			1.0f, -1.0f, 0.0f,		1.0f, 0.0f,
			0.0f, 1.0f, 0.0f,			0.5f, 1.0f
	};

	Graphics::cMesh* triangle = new Graphics::cMesh();

	// xyz * 4 + uv * 4 = 20
	triangle->CreateMesh(vertices, indices, 20, 12);
	s_renderList.push_back(triangle);

	Graphics::cMesh* triangle2 = new Graphics::cMesh();

	// xyz * 4 + uv * 4 = 20
	triangle2->CreateMesh(vertices, indices, 20, 12);
	s_renderList.push_back(triangle2);
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
void SetUpTextures()
{
	s_brickTexture = Graphics::cTexture("Contents/textures/brick.png");
	s_brickTexture.LoadTexture();
	s_woodTexture = Graphics::cTexture("Contents/textures/wood2.png");
	s_woodTexture.LoadTexture();
}


/**
	The previous part is global functions and variables
	-----------------------------------------------------------------------------------
	The following part is in cMyGame class
**/

bool cMyGame::Initialize(GLuint i_width, GLuint i_height)
{
	auto result = true;
	if (!(result = cApplication::Initialize(i_width, i_width))) {
		assert(false, "Failed to initialize Application!");
		return false;
	}
	s_myGameInstance = this;


	// Create triangle
	CreateTriangle();
	//Compile shaders
	CreateEffect();
	// create camera
	CreateCamera();

	SetUpTextures();

	return result;
}


void cMyGame::Run()
{
	// loop until window closed
	while (!m_shouldApplicationLoopExit)
	{
		glfwPollEvents();

		// clear window
		glClearColor(0.8f, 0.8f, 0.8f, 1.f);
		// A lot of things can be cleaned like color buffer, depth buffer, so we need to specify what to clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Draw
		for (int i = 0; i < s_renderList.size(); ++i)
		{
			s_effectList[0]->UseEffect();

			glm::mat4 model = glm::identity<glm::mat4>();
			model = glm::translate(model, glm::vec3(0, -0.4, -2.5f));
			model = glm::rotate(model, ToRadian(180), glm::vec3(0, 0, 1));
			model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));
			glUniformMatrix4fv(s_effectList[0]->GetModelMatrixUniformID(), 1, GL_FALSE, glm::value_ptr(model));
			glUniformMatrix4fv(s_effectList[0]->GetProjectionMatrixUniformID(), 1, GL_FALSE, glm::value_ptr(s_mainCamera->GetProjectionMatrix()));
			glUniformMatrix4fv(s_effectList[0]->GetViewMatrixUniformID(), 1, GL_FALSE, glm::value_ptr(s_mainCamera->GetViewMatrix()));

			// use texture for
			s_brickTexture.UseTexture();

			s_renderList[0]->Render();

			// second obj
			{
				glm::mat4 model = glm::identity<glm::mat4>();
				model = glm::translate(model, glm::vec3(0, 0.4, -2.5f));
				model = glm::rotate(model, ToRadian(0), glm::vec3(0, 0, 1));
				model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));
				glUniformMatrix4fv(s_effectList[0]->GetModelMatrixUniformID(), 1, GL_FALSE, glm::value_ptr(model));
				glUniformMatrix4fv(s_effectList[0]->GetProjectionMatrixUniformID(), 1, GL_FALSE, glm::value_ptr(s_mainCamera->GetProjectionMatrix()));
				glUniformMatrix4fv(s_effectList[0]->GetViewMatrixUniformID(), 1, GL_FALSE, glm::value_ptr(s_mainCamera->GetViewMatrix()));

				// use texture for
				s_woodTexture.UseTexture();

				s_renderList[1]->Render();
			}


			// clear program
			glUseProgram(0);
		}

		// ----------------------
		// Swap buffers
		m_window->SwapBuffers();
	}
}

void cMyGame::UpdateBasedOnTime(float DeltaSeconds)
{
	// get + handle user input events
	{
		sWindowInput* _windowInput = m_window->GetWindowInput();
		s_mainCamera->CameraControl(_windowInput, DeltaSeconds);
		s_mainCamera->MouseControl(_windowInput->DX(), _windowInput->DY(), DeltaSeconds);
	}
}


void cMyGame::CleanUp()
{
	// Clean up memories
	{
		delete s_mainCamera;
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

	cApplication::CleanUp();
}