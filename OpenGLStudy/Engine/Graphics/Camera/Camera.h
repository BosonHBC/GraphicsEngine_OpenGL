#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Math/Transform/Transform.h"

// Forward declaration
// ------------------------
struct sWindowInput;
class cCamera
{
public:
	/** Constructors and destructor */
	cCamera() : m_translationSpeed(1), m_turnSpeed(0.1f)
	{
		m_transform = new cTransform();
	}
	cCamera(glm::vec3 i_initialPos, GLfloat i_initialPitch = 0.0, GLfloat i_initialYaw = 0.0, GLfloat i_moveSpeed = 1.0, GLfloat i_turnSpeed = 1.0f):
		m_translationSpeed(i_moveSpeed), m_turnSpeed(i_turnSpeed)
	{
		m_transform = new cTransform();
		glm::quat _pitch(glm::vec3(i_initialPitch, 0, 0));
		glm::quat _yaw(glm::vec3(0, i_initialYaw, 0));
		m_transform->SetTransform(i_initialPos, _pitch * _yaw, glm::vec3(1, 1, 1));
	}
	virtual ~cCamera();

	/** Usage functions*/
	virtual void CameraControl(sWindowInput* const i_windowInput, float i_dt);
	virtual void MouseControl(sWindowInput* const i_windowInput, float i_dt);
	// Projection matrix
	void CreateProjectionMatrix(GLfloat i_fov, GLfloat i_aspect, GLfloat i_nearPlane = 0.1f, GLfloat i_farPlane = 100.f);

	// Update uniform locations
	void UpdateUniformLocation(GLuint i_programID);

	/** Getters*/
	glm::mat4 GetViewMatrix();
	glm::vec3 CamLocation() const { return m_transform->Position(); }

	const glm::mat4& GetProjectionMatrix() const { return m_projectionMatrix; }
protected:
	/** private member variables*/
	cTransform* m_transform;
	GLfloat m_translationSpeed;
	GLfloat m_turnSpeed;

	glm::mat4 m_viewMatrix;
	glm::mat4 m_projectionMatrix;

	/** private helper functions*/
	virtual void Update();


};

