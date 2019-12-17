#include "Camera.h"
#include "glfw/glfw3.h"
#include "Graphics/Window/WindowInput.h"
glm::vec3 cCamera::WorldUp = glm::vec3(0.0, 1.0, 0.0);
glm::vec3 cCamera::WorldRight = glm::vec3(1.0, 0.0, 0.0);
glm::vec3 cCamera::WorldForward = glm::vec3(0.0, 0.0, 1.0);

void cCamera::Update()
{
	// Clamp the domain of pitch and yaw
	m_pitch = glm::clamp(m_pitch, 0.f, 180.f);
	m_yaw = glm::clamp(m_yaw, 0.f, 360.f);

	m_forward = WorldUp * cos(glm::radians(m_pitch)) + sin(glm::radians(m_pitch)) * (-WorldForward * cos(glm::radians(m_yaw)) + WorldRight * sin(glm::radians(m_yaw))) * sin(m_pitch);

	m_right = glm::normalize(glm::cross(m_forward, WorldUp));
	m_up = glm::normalize(glm::cross(m_right, m_forward));
}



cCamera::~cCamera()
{

}

void cCamera::CameraControl(sWindowInput* const i_windowInput, float i_deltaSecs)
{
	if (i_windowInput->IsKeyDown(GLFW_KEY_W)) 
	{
		m_position += m_forward * m_translationSpeed * i_deltaSecs;
	}
	if (i_windowInput->IsKeyDown(GLFW_KEY_S))
	{
		m_position -= m_forward * m_translationSpeed * i_deltaSecs;
	}
	if (i_windowInput->IsKeyDown(GLFW_KEY_A))
	{
		m_position -= m_right * m_translationSpeed * i_deltaSecs;
	}
	if (i_windowInput->IsKeyDown(GLFW_KEY_D))
	{
		m_position += m_right * m_translationSpeed * i_deltaSecs;
	}
}

glm::mat4 cCamera::GetViewMatrix() const
{
	glm::vec3 _targetLoc = m_position + m_forward;
	return glm::lookAt(m_position, _targetLoc, m_up);
}

void cCamera::CreateProjectionMatrix(GLfloat i_fov, GLfloat i_aspect, GLfloat i_nearPlane /*= 0.1f*/, GLfloat i_farPlane /*= 100.f*/)
{
	m_projectionMatrix = glm::perspective(i_fov, i_aspect, i_nearPlane, i_farPlane);
}

