#include "Camera.h"
#include "glfw/glfw3.h"
#include "Application/Window/WindowInput.h"

void cCamera::Update()
{
	// Clamp the domain of pitch and yaw

	m_transform->Update();

}

void cCamera::UpdateUniformLocation(GLuint i_programID)
{

}

cCamera::~cCamera()
{

}

void cCamera::CameraControl(sWindowInput* const i_windowInput, float i_dt)
{
	if (i_windowInput->IsKeyDown(GLFW_KEY_W))
	{
		m_transform->Translate(m_transform->Forward() * m_translationSpeed * i_dt);
	}
	if (i_windowInput->IsKeyDown(GLFW_KEY_S))
	{
		m_transform->Translate(-m_transform->Forward() * m_translationSpeed * i_dt);
	}
	if (i_windowInput->IsKeyDown(GLFW_KEY_A))
	{
		m_transform->Translate(m_transform->Right() * m_translationSpeed * i_dt);
	}
	if (i_windowInput->IsKeyDown(GLFW_KEY_D))
	{
		m_transform->Translate(-m_transform->Right() * m_translationSpeed * i_dt);
	}
	Update();
}


void cCamera::MouseControl(sWindowInput* const i_windowInput, float i_dt)
{
	m_transform->Rotate(glm::vec3(0, 1, 0), i_windowInput->DX() * m_turnSpeed);
	m_transform->Rotate(glm::vec3(1, 0, 0), i_windowInput->DY() * m_turnSpeed);

	Update();
}

glm::mat4 cCamera::GetViewMatrix()
{
	glm::vec3 _targetLoc = m_transform->Position() + m_transform->Forward();
	m_viewMatrix = glm::lookAt(m_transform->Position(), _targetLoc, -m_transform->Up());
	return m_viewMatrix;
}

void cCamera::CreateProjectionMatrix(GLfloat i_fov, GLfloat i_aspect, GLfloat i_nearPlane /*= 0.1f*/, GLfloat i_farPlane /*= 100.f*/)
{
	m_projectionMatrix = glm::perspective(i_fov, i_aspect, i_nearPlane, i_farPlane);
}


