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

	}
	cCamera(glm::vec3 i_initialPos, GLfloat i_initialPitch = 0.0, GLfloat i_initialYaw = 0.0, GLfloat i_moveSpeed = 1.0, GLfloat i_turnSpeed = 1.0f):
		m_yaw(i_initialYaw), m_pitch(i_initialPitch), m_worldUp(glm::vec3(0,1.f,0)),
		m_translationSpeed(i_moveSpeed), m_turnSpeed(i_turnSpeed)
	{
		glm::quat _pitch(glm::vec3(i_initialPitch, 0, 0));
		glm::quat _yaw(glm::vec3(0, i_initialYaw, 0));
		Transform.SetTransform(i_initialPos, _pitch * _yaw, glm::vec3(1, 1, 1));
	}
	cCamera(const cCamera& i_other);
	cCamera& operator = (const cCamera& i_other);
	virtual ~cCamera();

	/** Usage functions*/
	virtual void CameraControl(sWindowInput* const i_windowInput, float i_dt);
	virtual void MouseControl(sWindowInput* const i_windowInput, float i_dt);
	// Projection matrix
	void CreateProjectionMatrix(GLfloat i_fov, GLfloat i_aspect, GLfloat i_nearPlane = 0.1f, GLfloat i_farPlane = 100.f);

	void MirrorAlongPlane(const cTransform& i_plane);

	// Update uniform locations
	void UpdateUniformLocation(GLuint i_programID);

	/** Getters*/
	glm::mat4 GetViewMatrix();
	glm::vec3 CamLocation() const { return Transform.Position(); }
	const glm::mat4& GetProjectionMatrix() const { return m_projectionMatrix; }

	cTransform Transform;
protected:
	/** private member variables*/

	GLfloat m_translationSpeed;
	GLfloat m_turnSpeed;

	glm::mat4 m_viewMatrix;
	glm::mat4 m_projectionMatrix;

	GLfloat m_pitch;
	GLfloat m_yaw;

	glm::vec3 m_worldUp;

	/** private helper functions*/
	virtual void Update();


};

