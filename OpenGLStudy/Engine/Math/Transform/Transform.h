#pragma once
#include "GL/glew.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Math/Quaternion/Quaternion.h"

class cTransform
{
public:
	/** Constructors&destructor and assignment operators*/
	cTransform(): m_position(glm::vec3(0,0,0)), m_rotation(cQuaternion(1,0,0,0)), m_scale(glm::vec3(1,1,1)) {}
	cTransform(const cTransform& i_other) : m_position(i_other.m_position), m_rotation(i_other.m_rotation), m_scale(i_other.m_scale) {}
	cTransform& operator = (const cTransform& i_other);
	~cTransform();

	/** Usage function*/
	void Translate(const glm::vec3& i_location);
	void Rotate(const glm::vec3& i_axis, const float& i_angle);
	void Scale(const glm::vec3& i_scale);

	/** Getters */
	glm::mat4 GetTranslationMatrix() const;
	glm::mat4 GetRotationMatrix() const;
	glm::mat4 GetScaleMatrix() const;
	glm::vec3 GetWorldLocation() const;
	glm::vec3 GetEulerAngle() const;
	const glm::mat4& M() const;
	const glm::mat4& MInv() const;
	const glm::mat4 TranspostInverse() const;
	const glm::mat4 MirrorAccordingTo(const cTransform&t);
	/** Helper functions*/
	bool HasScale() const;
	void Update();

#ifdef _DEBUG
	void PrintEulerAngle() const;
#endif // _DEBUG


private:
	/** private data*/
	glm::vec3 m_position;
	cQuaternion m_rotation;
	glm::vec3 m_scale;

};

