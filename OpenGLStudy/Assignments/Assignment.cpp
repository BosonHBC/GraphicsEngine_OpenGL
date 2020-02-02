#include "Assignment.h"
#include "Graphics/Window/Window.h"
#include "Graphics/Window/WindowInput.h"
#include "Constants/Constants.h"

#include "assert.h"

#include "GL/glew.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"


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
		assert(false, "Failed to initialize Application!");
		return false;
	}
	CreateActor();
	CreateEffect();
	CreateCamera();
	CreateLight();

	return result;
}
void Assignment::CreateEffect()
{
	Graphics::cEffect* defaultEffect = new Graphics::cEffect();
	if (!defaultEffect->CreateProgram(Constants::CONST_PATH_DEFAULT_VERTEXSHADER, Constants::CONST_PATH_BLINNPHONG_FRAGMENTSHADER)) {
		exit(1);
		return;
	}
	m_effectList.push_back(defaultEffect);
	m_currentEffect = m_effectList[0];
	// -----------------------------------
	// Create materials
	// -----------------------------------
	auto _material = new Graphics::cMaterial();
	_material->SetDiffuseIntensity(defaultEffect->GetProgramID(), Color(1,0.0f,0.0f));
	_material->SetSpecularIntensity(defaultEffect->GetProgramID(), Color(1,1,1));
	_material->SetShininess(defaultEffect->GetProgramID(), 32);

	// Set material
	m_teapot->SetMaterial(_material);
	// -----------------------------------
	// Create models
	// -----------------------------------
	Graphics::cModel* _teapot = new Graphics::cModel();
	_teapot->LoadModel("Contents/models/teapot.obj");

	// Set model
	m_teapot->SetModel(_teapot);


}

void Assignment::CreateActor()
{
	m_teapot = new cActor();
	m_teapot->Initialize();
	m_teapot->Transform()->Translate(glm::vec3(0, -0.4f, -2.f));
	m_teapot->Transform()->Rotate(glm::vec3(1, 0, 0), -90.f);
	m_teapot->Transform()->Scale(glm::vec3(0.05f, 0.05f, 0.05f));
}

void Assignment::CreateCamera()
{
	m_editorCamera = new cEditorCamera(glm::vec3(0,1.f,0),-30.f,0,3,10.f);
	float _aspect = (float)(Get_GLFW_Window()->GetBufferWidth()) / (float)(Get_GLFW_Window()->GetBufferHeight());
	m_editorCamera->CreateProjectionMatrix(45.0f, _aspect, 0.1f, 150.0f);
}

void Assignment::CreateLight()
{

	m_PointLight = new Graphics::cPointLight(Color(1.f, 1.f, 1.f), 0.3f, 0.1f, 0.1f);
	m_PointLight->SetupLight(m_currentEffect->GetProgramID(), 0);
	m_PointLight->SetLightInitialLocation(glm::vec3(0, 1.5f, 0));
	//Color(0.1f, 0.1f, 0.1f)
	m_ambientLight = new Graphics::cAmbientLight(Color::Black());
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
			m_currentEffect->SetPointLightCount(1);
			m_PointLight->Illuminate();
		}
		// main draw happens in this scope
		// --------------------------------------------------
		{
			m_teapot->Update(m_currentEffect);
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

	cApplication::CleanUp();
}

void Assignment::Tick(float second_since_lastFrame)
{
	sWindowInput* _windowInput = m_window->GetWindowInput();

	// get + handle user input events
	{
		m_editorCamera->CameraControl(_windowInput, second_since_lastFrame);

		m_editorCamera->MouseControl(_windowInput, 0.01667f);
	}

}

void Assignment::FixedTick()
{
	
}
