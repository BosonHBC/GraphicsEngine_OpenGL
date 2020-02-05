#include "Transform.h"

#include "Effect/Effect.h"
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

cTransform::~cTransform()
{
}

void cTransform::Translate(const glm::vec3& i_location)
{
	m = glm::translate(m, i_location);
	// mInv = ? // should not inverse the m every time it translate
}

void cTransform::Rotate(const glm::vec3& i_axis, const float& i_angle)
{
	m = glm::rotate(m, glm::radians(i_angle), i_axis);
}

void cTransform::Scale(const glm::vec3& i_scale)
{
	m = glm::scale(m, i_scale);
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

bool cTransform::HasScale() const
{
#define NOT_ONE(x) ((x) < .999f || (x) > 1.001f)
	return (NOT_ONE((m*glm::vec4(1, 0, 0, 0)).length()) || NOT_ONE((m*glm::vec4(1, 0, 0, 0)).length()) || NOT_ONE((m*glm::vec4(1, 0, 0, 0)).length()));
#undef NOT_ONE;
}

void cTransform::Update()
{
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
#endif
