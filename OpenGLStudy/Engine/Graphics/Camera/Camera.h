#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

// Forward declaration
// ------------------------
struct sWindowInput;
class cCamera
{
public:
	/** Constructors and destructor */
	cCamera() : m_position(glm::vec3(0, 0, 0)), m_forward(glm::vec3(0, 0, -1)), m_up(glm::vec3(0, 1, 0)),
		m_right(glm::vec3(1, 0, 0)), m_pitch(90.f), m_yaw(0),m_translationSpeed(1), m_turnSpeed(1)
	{
		Update();
	}
	cCamera(glm::vec3 i_initialPos, GLfloat i_initialPitch = 90.0, GLfloat i_initialYaw = 0.0, GLfloat i_moveSpeed = 1.0, GLfloat i_turnSpeed = 1.0):
		m_position(i_initialPos), m_forward(glm::vec3(0, 0, -1)), m_up(glm::vec3(0, 1, 0)), m_right(glm::vec3(1, 0, 0)),
		m_pitch(i_initialPitch), m_yaw(i_initialYaw),
		m_translationSpeed(i_moveSpeed), m_turnSpeed(i_turnSpeed)
	{
		Update();
	}
	~cCamera();

	/** Usage functions*/
	void CameraControl(sWindowInput* const i_windowInput, float i_deltaSecs);
	glm::mat4 GetViewMatrix() const;
	const glm::mat4& GetProjectionMatrix() const { return m_projectionMatrix; }
	// Projection matrix
	void CreateProjectionMatrix(GLfloat i_fov, GLfloat i_aspect, GLfloat i_nearPlane = 0.1f, GLfloat i_farPlane = 100.f);

private:
	/** private member variables*/
	glm::vec3 m_position;
	glm::vec3 m_forward;
	glm::vec3 m_up;
	glm::vec3 m_right;

	GLfloat m_pitch;
	GLfloat m_yaw;

	GLfloat m_translationSpeed;
	GLfloat m_turnSpeed;

	glm::mat4 m_projectionMatrix;

	/** private helper functions*/
	void Update();

	/** private static variables*/
	static glm::vec3 WorldUp;
	static glm::vec3 WorldRight;
	static glm::vec3 WorldForward;

};

