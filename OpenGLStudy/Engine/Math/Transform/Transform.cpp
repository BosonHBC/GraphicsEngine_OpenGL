#include "Transform.h"

cTransform& cTransform::operator=(const cTransform& i_other)
{
	m = i_other.m;
	mInv = i_other.mInv;
	return *this;
}

cTransform& cTransform::operator=(const glm::mat4& i_m)
{
	m = i_m;
	mInv = glm::inverse(m);
	return *this;
}

cTransform::cTransform(const glm::vec3& i_initialTranslation, const glm::quat& i_intialRotation, const glm::vec3& i_initialScale)
{
	SetTransform(i_initialTranslation, i_intialRotation, i_initialScale);
}

cTransform::~cTransform()
{
}

glm::quat cTransform::ToQuaternian(const double yaw, const double pitch, const double roll)
{
	// Abbreviations for the various angular functions
	double cy = cos(yaw * 0.5);
	double sy = sin(yaw * 0.5);
	double cp = cos(pitch * 0.5);
	double sp = sin(pitch * 0.5);
	double cr = cos(roll * 0.5);
	double sr = sin(roll * 0.5);

	glm::quat q;
	q.w = cy * cp * cr + sy * sp * sr;
	q.x = cy * cp * sr - sy * sp * cr;
	q.y = sy * cp * sr + cy * sp * cr;
	q.z = sy * cp * cr - cy * sp * sr;

	return q;
}

void cTransform::Translate(const glm::vec3& i_location)
{
	m_position += i_location;
}

void cTransform::Rotate(const glm::vec3& i_axis, const float& i_angle)
{
	m_rotation *= glm::angleAxis(i_angle, i_axis);
}

void cTransform::Scale(const glm::vec3& i_scale)
{
	m_scale *= i_scale;
}

void cTransform::gRotate(const glm::vec3& i_axis, const float& i_angle)
{
	glm::vec3 _worldAxis = glm::inverse(m_rotation) * i_axis;
	m_rotation *= glm::angleAxis(i_angle, _worldAxis);
}

void cTransform::gScale(const glm::vec3& i_scale)
{
	glm::vec3 _worldScale = glm::inverse(m_rotation) * i_scale;
	m_scale *= _worldScale;
}

void cTransform::SetTransform(const glm::vec3 & i_initialTranslation, const glm::quat & i_intialRotation, const glm::vec3 & i_initialScale)
{
	/*
		m = glm::toMat4(i_intialRotation);
		m[3] = glm::vec4(i_initialTranslation, 1);
		m = glm::scale(m, i_initialScale);

		mInv = glm::inverse(m);*/

	m_position = i_initialTranslation;
	m_rotation = i_intialRotation;
	m_scale = i_initialScale;
}


glm::vec3 cTransform::GetWorldLocation() const
{
	return glm::vec3(m[3][0], m[3][1], m[3][2]);
}

glm::vec3 cTransform::GetEulerAngle() const
{
	double sy = sqrt(m[0][0] * m[0][0] + m[1][0] * m[1][0]);

	bool singular = sy < 1e-6;

	float x, y, z;
	if (!singular)
	{
		x = atan2(m[2][1], m[2][2]);
		y = atan2(-m[2][0], sy);
		z = atan2(m[1][0], m[0][0]);
	}
	else
	{
		x = atan2(-m[1][2], m[1][1]);
		y = atan2(-m[2][0], sy);
		z = 0;
	}
	return glm::vec3(x, y, z);
}

glm::mat4 cTransform::GetTranslationMatrix() const
{
	glm::mat4 _m = glm::mat4(1.0);
	_m[3] = glm::vec4(m_position, 1);
	return _m;
}

glm::mat4 cTransform::GetRotationMatrix() const
{
	return glm::toMat4(m_rotation);
}

glm::mat4 cTransform::GetScaleMatrix() const
{
	glm::mat4 _m = glm::mat4(1.0);
	_m[0][0] = m_scale.x; _m[1][1] = m_scale.y; _m[2][2] = m_scale.z;
	return _m;

}

glm::vec3 cTransform::Forward() const
{
	return m_rotation * cTransform::WorldForward;
}

glm::vec3 cTransform::Right() const
{
	return m_rotation * cTransform::WorldRight;
}

glm::vec3 cTransform::Up() const
{
	return m_rotation * cTransform::WorldUp;
}

bool cTransform::HasScale() const
{
#define NOT_ONE(x) ((x) < .999f || (x) > 1.001f)
	return (NOT_ONE((m*glm::vec4(1, 0, 0, 0)).length()) || NOT_ONE((m*glm::vec4(1, 0, 0, 0)).length()) || NOT_ONE((m*glm::vec4(1, 0, 0, 0)).length()));
#undef NOT_ONE
}

void cTransform::Update()
{
	m = GetTranslationMatrix() * GetRotationMatrix() * GetScaleMatrix();
	mInv = glm::inverse(m);
}

#ifdef _DEBUG
#include "stdio.h"
#define ToDegree(x) (x*57.2958f)
void cTransform::PrintEulerAngle() const
{
	glm::vec3 angle = GetEulerAngle();

	printf("angle: %f, %f, %f\n", ToDegree(angle.x), ToDegree(angle.y), ToDegree(angle.z));
}

glm::vec3 cTransform::WorldUp = glm::vec3(0.0, 1.0, 0.0);

glm::vec3 cTransform::WorldRight = glm::vec3(1.0, 0.0, 0.0);

glm::vec3 cTransform::WorldForward = glm::vec3(0.0, 0.0, 1.0);


#endif
