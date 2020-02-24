#include "Camera.h"
#include "glfw/glfw3.h"
#include "Application/Window/WindowInput.h"

void cCamera::Update()
{
	// Clamp the domain of pitch and yaw
	m_pitch = glm::clamp(m_pitch, -89.f, 89.f);
	glm::vec3 _forward = cTransform::WorldUp * sin(glm::radians(m_pitch)) + cos(glm::radians(m_pitch)) * (-cTransform::WorldForward * cos(glm::radians(m_yaw)) + cTransform::WorldRight * sin(glm::radians(m_yaw)));
	glm::vec3 _right = glm::normalize(glm::cross(_forward, cTransform::WorldUp));
	glm::vec3 _up = glm::normalize(glm::cross(_right, _forward));

	m_transform->SetRotation(glm::quatLookAt(_forward, _up));
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
	Update();
}

glm::mat4 cCamera::GetViewMatrix()
{
	glm::vec3 _targetLoc = m_transform->Position() + m_transform->Forward();
	m_viewMatrix = glm::lookAt(m_transform->Position(), _targetLoc, cTransform::WorldUp);
	return m_viewMatrix;
}

void cCamera::CreateProjectionMatrix(GLfloat i_fov, GLfloat i_aspect, GLfloat i_nearPlane /*= 0.1f*/, GLfloat i_farPlane /*= 100.f*/)
{
	m_projectionMatrix = glm::perspective(i_fov, i_aspect, i_nearPlane, i_farPlane);
}


