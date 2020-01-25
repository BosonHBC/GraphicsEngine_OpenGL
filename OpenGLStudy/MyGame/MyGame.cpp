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
#include "Graphics/Light/SpotLight/SpotLight.h"

#include <assimp/Importer.hpp>

// constants definition
// -----------------------
#define ToRadian(x) x * 0.0174532925f

std::vector<Graphics::cMesh*> s_renderList = std::vector<Graphics::cMesh *>();
std::vector<Graphics::cEffect*> s_effectList = std::vector<Graphics::cEffect *>();
cCamera* s_mainCamera;
cMyGame* s_myGameInstance;

Graphics::cMaterial s_brickMat;
Graphics::cMaterial s_woodMat;
Graphics::cMaterial s_floorMat;

Graphics::cAmbientLight s_ambientLight;
Graphics::cDirectionalLight s_DirectionalLight;
const int s_pointLightCount = 2;
Graphics::cPointLight s_pointLights[s_pointLightCount];
const int s_spotLightCount = 1;
Graphics::cSpotLight s_spotLights[s_spotLightCount];

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

	Assimp::Importer importer;

	GLuint indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1 ,2
	};

	GLfloat vertices[] = {
		//     x,		y,		z,			u,		v			n_x, 	n_y,	n_z
			-1.0f, -1.0f, -0.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 1.732f,		0.5f, 0.0f,		0.0f, 0.0f, 0.0f,
			1.0f, -1.0f, 0.0f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.5773f,			0.5f, 1.0f,		0.0f, 0.0f, 0.0f
	};

	GLuint floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat floorVertices[] = {
		//     x,		y,		z,			u,		v			n_x, 	n_y,	n_z
			-10.f, 0.0f, -10.0f,		0.0f, 0.0f,		0.0f, -1.0f,  0.0f,
			10.f, 0.0f, -10.0f,		10.f, 0.0f,		0.0f, -1.0f, 0.0f,
			-10.f, 0.0f, 10.0f,		0.0f, 10.f,		0.0f, -1.0f, 0.0f,
			10.f,	0.0f, 10.0f,		10.f, 10.f,		0.0f, -1.0f, 0.0f,
	};



	CalculateAverageNormals(indices, 12, vertices, 32, 8, 5);

	Graphics::cMesh* triangle = new Graphics::cMesh();
	triangle->CreateMesh(vertices, indices, 32, 12);
	s_renderList.push_back(triangle);

	Graphics::cMesh* triangle2 = new Graphics::cMesh();
	triangle2->CreateMesh(vertices, indices, 32, 12);
	s_renderList.push_back(triangle2);

	Graphics::cMesh* floor = new Graphics::cMesh();
	floor->CreateMesh(floorVertices, floorIndices, 32, 6);
	s_renderList.push_back(floor);

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
	s_brickMat.SetDiffuse("Contents/textures/brick.png");
	s_brickMat.SetShininess(4);

	s_woodMat.SetDiffuse("Contents/textures/wood2.png");
	s_woodMat.SetShininess(64);
	
	s_floorMat.SetDiffuse("Contents/textures/whiteBoard.png");
	s_floorMat.SetShininess(128);
}
void SetUpLights()
{
	// Ambient Light
	s_ambientLight = Graphics::cAmbientLight(0.05f, 0.0f, Color(1, 1, 1));
	s_ambientLight.SetupLight(s_effectList[0]->GetProgramID());

	// Directional light
	s_DirectionalLight = Graphics::cDirectionalLight(0.2f, 0.8f, Color(1, 1, 0.9f)
														, glm::vec3(0, -1, -1));
	s_DirectionalLight.SetupLight(s_effectList[0]->GetProgramID());

	s_pointLights[0] = Graphics::cPointLight(0.3f, 0.75f, Color(0.8f, 0.2f, 0.2f)
														,glm::vec3(-2.5f, 1.5f, 0.3f), 0.3f, 0.1f, 0.1f);
	s_pointLights[0].SetupLight(s_effectList[0]->GetProgramID(), 0);

	s_pointLights[1] = Graphics::cPointLight(0.5f, 1.f, Color(0.2f, 0.8f, 0.2f)
														,glm::vec3(2.5f, 1.5f, 0.3f), 0.3f, 0.2f, 0.1f);
	s_pointLights[1].SetupLight( s_effectList[0]->GetProgramID(), 1);

	s_spotLights[0] = Graphics::cSpotLight(0.5f, 1.f, Color(1, 1, 1), glm::vec3(0,1, 0) 
														, glm::vec3(5, -1, 0), 20.f, 1.f, 0.0f, 0.0f);
	s_spotLights[0].SetupLight(s_effectList[0]->GetProgramID(), 0);
}

/**
	The previous part is global functions and variables
	-----------------------------------------------------------------------------------
	The following part is in cMyGame class
**/

bool cMyGame::Initialize(GLuint i_width, GLuint i_height, const char* i_windowName)
{
	auto result = true;
	if (!(result = cApplication::Initialize(i_width, i_width, i_windowName))) {
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
		glClearColor(0,0,0, 1.f);
		// A lot of things can be cleaned like color buffer, depth buffer, so we need to specify what to clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/** Drawing Starts*/ 
		// Use this programID
		s_effectList[0]->UseEffect();
		// s_pointLightCount must be correct
		s_effectList[0]->SetPointLightCount(s_pointLightCount);
		s_effectList[0]->SetSpotLightCount(s_spotLightCount);
		// Illuminate the light
		//s_ambientLight.Illuminate();
		//s_DirectionalLight.Illuminate();
		for (int i = 0; i < s_pointLightCount; ++i)
		{
			s_pointLights[i].Illuminate();
		}
		// Update spot light position
		s_spotLights[0].SetSpotLight(s_mainCamera->CamLocation() + glm::vec3(0, -0.3f, 0) + 0.3f * s_mainCamera->CamRight(), s_mainCamera->CamForward());
		for (int i = 0; i < s_spotLightCount; ++i)
		{
			s_spotLights[i].Illuminate();
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
		// floor obj
		{
			glm::mat4 model = glm::identity<glm::mat4>();
			model = glm::translate(model, glm::vec3(0, -1.f, 0.0f));

			glUniformMatrix4fv(s_effectList[0]->GetModelMatrixUniformID(), 1, GL_FALSE, glm::value_ptr(model));
			glUniformMatrix4fv(s_effectList[0]->GetProjectionMatrixUniformID(), 1, GL_FALSE, glm::value_ptr(s_mainCamera->GetProjectionMatrix()));
			glUniformMatrix4fv(s_effectList[0]->GetViewMatrixUniformID(), 1, GL_FALSE, glm::value_ptr(s_mainCamera->GetViewMatrix()));

			// use texture for floor
			s_floorMat.UseMaterial(s_effectList[0]->GetProgramID());

			s_renderList[2]->Render();
		}

		// clear program
		glUseProgram(0);

		// ----------------------
		// Swap buffers
		m_window->SwapBuffers();
	}
}

void cMyGame::Tick(float second_since_lastFrame)
{
	// get + handle user input events
	{
		sWindowInput* _windowInput = m_window->GetWindowInput();
		s_mainCamera->CameraControl(_windowInput, second_since_lastFrame);
		s_mainCamera->MouseControl(_windowInput->DX(), _windowInput->DY(), second_since_lastFrame);
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

		s_woodMat.CleanUp();
		s_brickMat.CleanUp();
		s_floorMat.CleanUp();
	}

	cApplication::CleanUp();
}