#include "Assignment.h"
#include "Application/Window/Window.h"
#include "Application/Window/WindowInput.h"
#include "Constants/Constants.h"

#include "assert.h"

#include "GL/glew.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Time/Time.h"

#include "Graphics/Effect/Effect.h"
#include "Graphics/Camera/EditorCamera/EditorCamera.h"
#include "Graphics/Model/Model.h"
#include "Light/PointLight/PointLight.h"
#include "Light/AmbientLight/AmbientLight.h"
#include "Material/Material.h"
#include "Actor/Actor.h"
#include "Transform/Transform.h"


bool Assignment::Initialize(GLuint i_width, GLuint i_height, const char* i_windowName /*= "Default Window"*/)
{
	auto result = true;
	if (!(result = cApplication::Initialize(i_width, i_width, i_windowName))) {
		assert(false);
		// TODO: LogError
		printf("Failed to initialize Application!");
		return false;
	}
	CreateEffect();
	CreateActor();

	CreateCamera();
	CreateLight();

	return result;
}
void Assignment::CreateEffect()
{
	Graphics::cEffect* defaultEffect = new  Graphics::cEffect();
	if (!defaultEffect->CreateProgram(Constants::CONST_PATH_DEFAULT_VERTEXSHADER, Constants::CONST_PATH_BLINNPHONG_FRAGMENTSHADER)) {
		exit(1);
		return;
	}
	m_effectList.push_back(defaultEffect);
	m_currentEffect = m_effectList[0];

}

void Assignment::CreateActor()
{
	m_teapot = new cActor();
	m_teapot->Initialize();
	m_teapot->Transform()->Translate(glm::vec3(0, -0.4f, -2.f));
	m_teapot->Transform()->Rotate(glm::vec3(1, 0, 0), -90.f);
	m_teapot->Transform()->Scale(glm::vec3(0.05f, 0.05f, 0.05f));

	m_teapot->SetModel("Contents/models/teapot.model");
	m_teapot->UpdateUniformVariables(m_currentEffect);

	m_teapot2 = new cActor();
	m_teapot2->Initialize();
	m_teapot2->Transform()->Translate(glm::vec3(2, -0.4f, -3.f));
	m_teapot2->Transform()->Rotate(glm::vec3(1, 0, 0), -90.f);
	m_teapot2->Transform()->Scale(glm::vec3(0.03f, 0.03f, 0.03f));

	m_teapot2->SetModel("Contents/models/teapot.model");
	m_teapot2->UpdateUniformVariables(m_currentEffect);

	m_plane = new cActor();
	m_plane->Initialize();
	m_plane->SetModel("Contents/models/plane.model");
	m_plane->UpdateUniformVariables(m_currentEffect);
	m_plane->Transform()->Translate(glm::vec3(0,-0.4f, -2.f));
	m_plane->Transform()->Scale(glm::vec3(5, 1, 5));

}

void Assignment::CreateCamera()
{
	m_editorCamera = new  cEditorCamera(glm::vec3(0, 1.f, 0), -30.f, 0, 3, 10.f);
	float _aspect = (float)(Get_GLFW_Window()->GetBufferWidth()) / (float)(Get_GLFW_Window()->GetBufferHeight());
	m_editorCamera->CreateProjectionMatrix(45.0f, _aspect, 0.1f, 150.0f);
}

void Assignment::CreateLight()
{

	m_PointLight = new  Graphics::cPointLight(Color(1.f, 1.f, 1.f), 0.3f, 0.1f, 0.1f);
	m_PointLight->SetupLight(m_currentEffect->GetProgramID(), 0);
	m_PointLight->SetLightInitialLocation(glm::vec3(0, 1.5f, 0));

	m_PointLight2 = new Graphics::cPointLight(Color(1, 1, 1), 0.5f, 0.2f, 0.1f);
	m_PointLight2->SetupLight(m_currentEffect->GetProgramID(), 1);
	m_PointLight2->SetLightInitialLocation(glm::vec3(-3, 0, -3));

	m_ambientLight = new  Graphics::cAmbientLight(Color(0.1f, 0.1f, 0.1f));
	m_ambientLight->SetupLight(m_currentEffect->GetProgramID(), 0);
}

void Assignment::Run()
{

	// loop until window closed
	while (!m_shouldApplicationLoopExit)
	{
		// clear window
		glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, 1.f);
		// A lot of things can be cleaned like color buffer, depth buffer, so we need to specify what to clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glfwPollEvents();

		// for recompile shader
		if (m_window->GetWindowInput()->IsKeyDown(GLFW_KEY_F6)) {
			m_currentEffect->RecompileShader(Constants::CONST_PATH_DEFAULT_VERTEXSHADER, GL_VERTEX_SHADER);
		}

		m_currentEffect->UseEffect();

		// Update camera location
		// --------------------------------------------------
		m_editorCamera->UpdateUniformLocation(m_currentEffect->GetProgramID());
		glUniformMatrix4fv(m_currentEffect->GetProjectionMatrixUniformID(), 1, GL_FALSE, glm::value_ptr(m_editorCamera->GetProjectionMatrix()));
		glUniformMatrix4fv(m_currentEffect->GetViewMatrixUniformID(), 1, GL_FALSE, glm::value_ptr(m_editorCamera->GetViewMatrix()));

		// Set up lighting
		// --------------------------------------------------
		{
			m_ambientLight->Illuminate();
			m_currentEffect->SetPointLightCount(2);
			m_PointLight->Illuminate();
			m_PointLight2->Illuminate();
		}
		// main draw happens in this scope
		// --------------------------------------------------
		{
			m_teapot->Update(m_currentEffect);
			m_teapot2->Update(m_currentEffect);
			m_plane->Update(m_currentEffect);
		}
		// --------------------------------------------------

		// clear program
		glUseProgram(0);

		// ----------------------
		// Swap buffers
		m_window->SwapBuffers();
	}
}

void Assignment::CleanUp()
{

	for (auto it = m_effectList.begin(); it != m_effectList.end(); ++it)
	{
		safe_delete(*it);
	}
	m_effectList.clear();
	m_effectList.~vector();

	safe_delete(m_editorCamera);
	safe_delete(m_PointLight);
	safe_delete(m_ambientLight);
	safe_delete(m_teapot);
	safe_delete(m_teapot2);
	safe_delete(m_plane);

}

void Assignment::Tick(float second_since_lastFrame)
{
	sWindowInput* _windowInput = m_window->GetWindowInput();

	// get + handle user input events
	{
		m_editorCamera->CameraControl(_windowInput, second_since_lastFrame);

		m_editorCamera->MouseControl(_windowInput, 0.01667f);
	}


	glm::vec3 _toLight = (m_PointLight->Transform()->GetWorldLocation() - m_teapot->Transform()->GetWorldLocation());
	glm::vec3 _toForward = glm::normalize(glm::cross(glm::vec3(0, 1, 0), _toLight)) * (5 * second_since_lastFrame);
	m_PointLight->Transform()->Translate(_toForward);

	/*
		m_teapot->Transform()->PrintEulerAngle();

		if (_windowInput->IsKeyDown(GLFW_KEY_LEFT)) {
			m_teapot->Transform()->Rotate(glm::vec3(0, 0, 1), (50 * second_since_lastFrame));
		}
		if (_windowInput->IsKeyDown(GLFW_KEY_RIGHT)) {
			m_teapot->Transform()->Rotate(glm::vec3(0, 0, 1), (-50 * second_since_lastFrame));
		}
		if (_windowInput->IsKeyDown(GLFW_KEY_UP)) {
			m_teapot->Transform()->Rotate(glm::vec3(1, 0, 0), (50 * second_since_lastFrame));
		}
		if (_windowInput->IsKeyDown(GLFW_KEY_DOWN)) {
			m_teapot->Transform()->Rotate(glm::vec3(1, 0, 0), (-50 * second_since_lastFrame));
		}*/
}

void Assignment::FixedTick()
{

}
