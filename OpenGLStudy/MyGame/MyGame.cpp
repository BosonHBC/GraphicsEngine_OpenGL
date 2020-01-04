#include "MyGame.h"

#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Graphics/Window/WindowInput.h"
#include "Graphics/Window/Window.h"
#include "Graphics/Mesh/Mesh.h"
#include "Graphics/Material/Material.h"
#include "Graphics/Effect/Effect.h"
#include "Graphics/Camera/Camera.h"
#include "Graphics/Light/AmbientLight/AmbientLight.h"
#include "Graphics/Light/DirectionalLight/DirectionalLight.h"
#include "Graphics/Light/PointLight/PointLight.h"
// constants definition
// -----------------------
#define ToRadian(x) x * 0.0174532925f

GLFWwindow* s_mainWindow;

std::vector<Graphics::cMesh*> s_renderList = std::vector<Graphics::cMesh *>();
std::vector<Graphics::cEffect*> s_effectList = std::vector<Graphics::cEffect *>();
cCamera* s_mainCamera;
cMyGame* s_myGameInstance;

Graphics::cMaterial s_brickMat;
Graphics::cMaterial s_woodMat;

Graphics::cAmbientLight s_ambientLight;
Graphics::cDirectionalLight s_DirectionalLight;
const int s_pointLightCount = 2;
Graphics::cPointLight s_pointLights[s_pointLightCount];

// Function definition
// ----------------------------
void CalculateAverageNormals(GLuint* indices, GLuint indiceCount, GLfloat* vertices, GLuint vertexCount, GLuint sizeOfDataPerVertex, GLuint normalOffset);
void CreateEffect();
void CreateTriangle();
void CreateCamera();
void SetUpTextures();
void SetUpLights();

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
		//     x,		y,		z,			u,		v			n_x 	n_y,	n_z
			-1.0f, -1.0f, -0.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 1.732f,		0.5f, 0.0f,		0.0f, 0.0f, 0.0f,
			1.0f, -1.0f, 0.0f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.5773f,			0.5f, 1.0f,		0.0f, 0.0f, 0.0f
	};

	CalculateAverageNormals(indices, 12, vertices, 32, 8, 5);

	Graphics::cMesh* triangle = new Graphics::cMesh();

	// xyz * 4 + uv * 4 = 20
	triangle->CreateMesh(vertices, indices, 32, 12);
	s_renderList.push_back(triangle);

	Graphics::cMesh* triangle2 = new Graphics::cMesh();

	// xyz * 4 + uv * 4 = 20
	triangle2->CreateMesh(vertices, indices, 32, 12);
	s_renderList.push_back(triangle2);
}

void CalculateAverageNormals(GLuint* indices, GLuint indiceCount, GLfloat* vertices, GLuint vertexCount
	, GLuint sizeOfDataPerVertex, GLuint normalOffset)
{
	for (size_t i = 0; i < indiceCount; i += 3)
	{
		GLuint in0 = indices[i] * sizeOfDataPerVertex;
		GLuint in1 = indices[i + 1] * sizeOfDataPerVertex;
		GLuint in2 = indices[i + 2] * sizeOfDataPerVertex;

		glm::vec3 v1 = glm::vec3(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2 = glm::vec3(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 vN = glm::normalize(glm::cross(v1, v2));

		in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
		vertices[in0] += vN.x; vertices[in0 + 1] += vN.y; vertices[in0 + 2] += vN.z;
		vertices[in1] += vN.x; vertices[in1 + 1] += vN.y; vertices[in1 + 2] += vN.z;
		vertices[in2] += vN.x; vertices[in2 + 1] += vN.y; vertices[in2 + 2] += vN.z;
	}

	for (size_t i = 0; i < vertexCount / sizeOfDataPerVertex; ++i)
	{
		size_t _normalOffset = i * sizeOfDataPerVertex + normalOffset;
		glm::vec3 vec(vertices[_normalOffset], vertices[_normalOffset + 1], vertices[_normalOffset + 2]);
		vec = glm::normalize(vec);
		vertices[_normalOffset] = vec.x;
		vertices[_normalOffset + 1] = vec.y;
		vertices[_normalOffset + 2] = vec.z;
	}
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
	s_brickMat = Graphics::cMaterial();
	s_brickMat.SetDiffuse("Contents/textures/brick.png");
	s_brickMat.SetShininess(4);
	s_woodMat = Graphics::cMaterial();
	s_woodMat.SetDiffuse("Contents/textures/wood2.png");
	s_woodMat.SetShininess(32);
}
void SetUpLights()
{
	// Point Light
	s_ambientLight = Graphics::cAmbientLight(0.05f, 0.0f, glm::vec3(1,1,1));
	s_ambientLight.SetupLight(s_effectList[0]->GetProgramID());

	// Directional light
	s_DirectionalLight = Graphics::cDirectionalLight(0.2f, 0.8f, glm::vec3(1, 1, 0.9f)
														, glm::vec3(0, -1, -1));
	s_DirectionalLight.SetupLight(s_effectList[0]->GetProgramID());

	s_pointLights[0] = Graphics::cPointLight(0.3f, 0.75f,glm::vec3(0.8f, 0.2f, 0.2f)
														,glm::vec3(-1.5f, 1.5f, 0.3f), 1.f, 0.007f, 0.0002f);
	s_pointLights[0].SetupLight(s_effectList[0]->GetProgramID(), 0);

	s_pointLights[1] = Graphics::cPointLight(0.5f, 0.4f,glm::vec3(0.2f, 0.8f, 0.2f)
														,glm::vec3(1.5f, 1.5f, 0.3f), 1.f, 0.07f, 0.017f);
	s_pointLights[1].SetupLight( s_effectList[0]->GetProgramID(), 1);
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

	SetUpLights();

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

		/** Drawing Starts*/ 
		// Use this programID
		s_effectList[0]->UseEffect();
		// s_pointLightCount must be correct
		s_effectList[0]->SetPointLightCount(s_pointLightCount);
		// Illuminate the light
		s_ambientLight.Illuminate();
		s_DirectionalLight.Illuminate();
		for (int i = 0; i < s_pointLightCount; ++i)
		{
			s_pointLights[i].Illuminate();
		}
		// update camera
		s_mainCamera->UpdateUniformLocation(s_effectList[0]->GetProgramID());

		glm::mat4 model = glm::identity<glm::mat4>();
		model = glm::translate(model, glm::vec3(0, -0.4, -2.5f));
		model = glm::rotate(model, ToRadian(180), glm::vec3(0, 0, 1));
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
		glUniformMatrix4fv(s_effectList[0]->GetModelMatrixUniformID(), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(s_effectList[0]->GetProjectionMatrixUniformID(), 1, GL_FALSE, glm::value_ptr(s_mainCamera->GetProjectionMatrix()));
		glUniformMatrix4fv(s_effectList[0]->GetViewMatrixUniformID(), 1, GL_FALSE, glm::value_ptr(s_mainCamera->GetViewMatrix()));

		// use texture for first object
		s_brickMat.UseMaterial(s_effectList[0]->GetProgramID());

		s_renderList[0]->Render();

		// second obj
		{
			glm::mat4 model = glm::identity<glm::mat4>();
			model = glm::translate(model, glm::vec3(0, 0.4, -2.5f));
			model = glm::rotate(model, ToRadian(0), glm::vec3(0, 0, 1));
			model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
			glUniformMatrix4fv(s_effectList[0]->GetModelMatrixUniformID(), 1, GL_FALSE, glm::value_ptr(model));
			glUniformMatrix4fv(s_effectList[0]->GetProjectionMatrixUniformID(), 1, GL_FALSE, glm::value_ptr(s_mainCamera->GetProjectionMatrix()));
			glUniformMatrix4fv(s_effectList[0]->GetViewMatrixUniformID(), 1, GL_FALSE, glm::value_ptr(s_mainCamera->GetViewMatrix()));

			// use texture for second object
			s_woodMat.UseMaterial(s_effectList[0]->GetProgramID());

			s_renderList[1]->Render();
		}


		// clear program
		glUseProgram(0);

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