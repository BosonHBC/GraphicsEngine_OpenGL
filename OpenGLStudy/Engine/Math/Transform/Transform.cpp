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

bool cTransform::HasScale() const
{
#define NOT_ONE(x) ((x) < .999f || (x) > 1.001f)
	return (NOT_ONE((m*glm::vec4(1, 0, 0, 0)).length()) || NOT_ONE((m*glm::vec4(1, 0, 0, 0)).length()) || NOT_ONE((m*glm::vec4(1, 0, 0, 0)).length()));
#undef NOT_ONE;
}

void cTransform::Update()
{
	//mInv = glm::inverse(m);

}
