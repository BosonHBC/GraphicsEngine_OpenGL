#include "Camera.h"
#include "glfw/glfw3.h"
#include "Application/Window/WindowInput.h"
glm::vec3 cCamera::WorldUp = glm::vec3(0.0, 1.0, 0.0);
glm::vec3 cCamera::WorldRight = glm::vec3(1.0, 0.0, 0.0);
glm::vec3 cCamera::WorldForward = glm::vec3(0.0, 0.0, 1.0);

void cCamera::Update()
{
	// Clamp the domain of pitch and yaw

	m_pitch = glm::clamp(m_pitch, -89.f, 89.f);

	m_forward = WorldUp * sin(glm::radians(m_pitch)) + cos(glm::radians(m_pitch)) * (-WorldForward * cos(glm::radians(m_yaw)) + WorldRight * sin(glm::radians(m_yaw)));

	m_right = glm::normalize(glm::cross(m_forward, WorldUp));
	m_up = glm::normalize(glm::cross(m_right, m_forward));

}

void cCamera::UpdateUniformLocation(GLuint i_programID)
{
	GLuint m_camPositionLocation = glGetUniformLocation(i_programID, "camPos"); 
	glUniform3f(m_camPositionLocation , m_position.x, m_position.y, m_position.z);
}

cCamera::~cCamera()
{

}

void cCamera::CameraControl(sWindowInput* const i_windowInput, float i_dt)
{
	if (i_windowInput->IsKeyDown(GLFW_KEY_W))
	{
		m_position += m_forward * m_translationSpeed * i_dt;
	}
	if (i_windowInput->IsKeyDown(GLFW_KEY_S))
	{
		m_position -= m_forward * m_translationSpeed * i_dt;
	}
	if (i_windowInput->IsKeyDown(GLFW_KEY_A))
	{
		m_position -= m_right * m_translationSpeed * i_dt;
	}
	if (i_windowInput->IsKeyDown(GLFW_KEY_D))
	{
		m_position += m_right * m_translationSpeed * i_dt;
	}

}


void cCamera::MouseControl(sWindowInput* const i_windowInput, float i_dt)
{
	m_yaw += i_windowInput->DX() * m_turnSpeed ;
	m_pitch += i_windowInput->DY() * m_turnSpeed;
	Update();
}

glm::mat4 cCamera::GetViewMatrix()
{
	glm::vec3 _targetLoc = m_position + m_forward;
	m_viewMatrix = glm::lookAt(m_position, _targetLoc, m_up);
	return m_viewMatrix;
}

void cCamera::CreateProjectionMatrix(GLfloat i_fov, GLfloat i_aspect, GLfloat i_nearPlane /*= 0.1f*/, GLfloat i_farPlane /*= 100.f*/)
{
	m_projectionMatrix = glm::perspective(i_fov, i_aspect, i_nearPlane, i_farPlane);
}


