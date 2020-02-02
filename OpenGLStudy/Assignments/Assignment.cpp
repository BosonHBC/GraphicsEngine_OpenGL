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



bool Assignment::Initialize(GLuint i_width, GLuint i_height, const char* i_windowName /*= "Default Window"*/)
{
	auto result = true;
	if (!(result = cApplication::Initialize(i_width, i_width, i_windowName))) {
		assert(false, "Failed to initialize Application!");
		return false;
	}

	m_teapot = new Graphics::cModel();
	m_teapot->LoadModel("Contents/models/teapot.obj");
	CreateEffect();
	CreateCamera();

	m_PointLight = new Graphics::cPointLight(Color(1.f,1.f,1.f)
		, glm::vec3(0, 1.5f, 0), 0.3f, 0.1f, 0.1f);
	m_PointLight->SetupLight(m_effectList[0]->GetProgramID(), 0);
	//Color(0.1f, 0.1f, 0.1f)
	m_ambientLight = new Graphics::cAmbientLight(Color::Black());
	m_ambientLight->SetupLight(m_effectList[0]->GetProgramID(), 0);

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

	// -----------------------------------
	// Create materials
	// -----------------------------------
	m_material = new Graphics::cMaterial();
	m_material->SetDiffuseIntensity(defaultEffect->GetProgramID(), Color(1,0.0f,0.0f));
	m_material->SetSpecularIntensity(defaultEffect->GetProgramID(), Color(1,1,1));
	m_material->SetShininess(defaultEffect->GetProgramID(), 32);

}

void Assignment::CreateCamera()
{
	m_editorCamera = new cEditorCamera(glm::vec3(0,1.f,0),-30.f,0,3,10.f);
	float _aspect = (float)(Get_GLFW_Window()->GetBufferWidth()) / (float)(Get_GLFW_Window()->GetBufferHeight());
	m_editorCamera->CreateProjectionMatrix(45.0f, _aspect, 0.1f, 150.0f);
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
			m_effectList[0]->RecompileShader(Constants::CONST_PATH_DEFAULT_VERTEXSHADER, GL_VERTEX_SHADER);
		}

		m_effectList[0]->UseEffect();

		// Update camera location
		// --------------------------------------------------
		m_editorCamera->UpdateUniformLocation(m_effectList[0]->GetProgramID());

		// Set up lighting
		// --------------------------------------------------
		{	
			m_ambientLight->Illuminate();
			m_effectList[0]->SetPointLightCount(1);
			m_PointLight->Illuminate();
		}

		// Set up materials
		// --------------------------------------------------
		m_material->UseMaterial(m_effectList[0]->GetProgramID());

		// main draw happens in this scope
		// --------------------------------------------------
		{
			glm::mat4 model = glm::identity<glm::mat4>();
			model = glm::translate(model, glm::vec3(0, -0.4f, -2.f));
			model = glm::rotate(model, glm::radians(-90.f), glm::vec3(1, 0, 0));
			model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));

			glUniformMatrix4fv(m_effectList[0]->GetModelMatrixUniformID(), 1, GL_FALSE, glm::value_ptr(model));
			glUniformMatrix4fv(m_effectList[0]->GetProjectionMatrixUniformID(), 1, GL_FALSE, glm::value_ptr(m_editorCamera->GetProjectionMatrix()));
			glUniformMatrix4fv(m_effectList[0]->GetViewMatrixUniformID(), 1, GL_FALSE, glm::value_ptr(m_editorCamera->GetViewMatrix()));
			{
				// fix non-uniform scale
				glm::mat4 normalMatrix = glm::transpose(glm::inverse(model));
				glUniformMatrix4fv(m_effectList[0]->GetNormalMatrixUniformID(), 1, GL_FALSE, glm::value_ptr(normalMatrix));
			}
			m_teapot->Render();
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
	safe_delete(m_teapot);
	safe_delete(m_material);
	safe_delete(m_ambientLight);

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
