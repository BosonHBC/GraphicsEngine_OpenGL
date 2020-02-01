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
		m_right(glm::vec3(1, 0, 0)), m_pitch(0), m_yaw(0),m_translationSpeed(1), m_turnSpeed(0.1f)
	{
		Update();
	}
	cCamera(glm::vec3 i_initialPos, GLfloat i_initialPitch = 0.0, GLfloat i_initialYaw = 0.0, GLfloat i_moveSpeed = 1.0, GLfloat i_turnSpeed = 1.0f):
		m_position(i_initialPos), m_forward(glm::vec3(0, 0, -1)), m_up(glm::vec3(0, 1, 0)), m_right(glm::vec3(1, 0, 0)),
		m_pitch(i_initialPitch), m_yaw(i_initialYaw),
		m_translationSpeed(i_moveSpeed), m_turnSpeed(i_turnSpeed)
	{
		Update();
	}
	~cCamera();

	/** Usage functions*/
	virtual void CameraControl(sWindowInput* const i_windowInput, float i_dt);
	virtual void MouseControl(sWindowInput* const i_windowInput, float i_dt);
	// Projection matrix
	void CreateProjectionMatrix(GLfloat i_fov, GLfloat i_aspect, GLfloat i_nearPlane = 0.1f, GLfloat i_farPlane = 100.f);

	// Update uniform locations
	void UpdateUniformLocation(GLuint i_programID);

	/** Getters*/
	glm::mat4 GetViewMatrix() const;
	glm::vec3 CamLocation() const { return m_position; }
	glm::vec3 CamForward() const { return m_forward; }
	glm::vec3 CamRight() const { return m_right; }
	glm::vec3 CamUp() const { return m_up; }
	const glm::mat4& GetProjectionMatrix() const { return m_projectionMatrix; }
protected:
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

	GLuint m_camPositionLocation;

	/** private helper functions*/
	virtual void Update();

	/** private static variables*/
	static glm::vec3 WorldUp;
	static glm::vec3 WorldRight;
	static glm::vec3 WorldForward;

};

