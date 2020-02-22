#include "Transform.h"

cTransform& cTransform::operator=(const cTransform& i_other)
{
	m_position = i_other.m_position;
	m_rotation = i_other.m_rotation;
	m_scale = i_other.m_scale;
	return *this;
}

cTransform::~cTransform()
{
	m_position = glm::vec3(0, 0, 0);
	m_rotation = cQuaternion(1, 0, 0, 0);
	m_scale = glm::vec3(1, 1, 1);
}

void cTransform::Translate(const glm::vec3& i_location)
{
	m_position += i_location;

	// mInv = ? // should not inverse the m every time it translate
}

void cTransform::Rotate(const glm::vec3& i_axis, const float& i_angle)
{
	m_rotation *= cQuaternion(i_axis, i_angle);
}

void cTransform::Scale(const glm::vec3& i_scale)
{
	m_scale *= i_scale;
}

glm::mat4 cTransform::GetTranslationMatrix() const
{
	glm::mat4 m = glm::identity<glm::mat4>();
	m[3][0] = m_position.x;
	m[3][1] = m_position.y;
	m[3][2] = m_position.z;
	return glm::identity<glm::mat4>();
}

glm::mat4 cTransform::GetRotationMatrix() const
{
	return glm::identity<glm::mat4>();

	//return m_rotation.ToRotationMatrix();
}

glm::mat4 cTransform::GetScaleMatrix() const
{
	glm::mat4 m = glm::identity<glm::mat4>();
	m[0][0] = m_scale.x;
	m[1][1] = m_scale.y;
	m[2][2] = m_scale.z;
	return glm::identity<glm::mat4>();
}

glm::vec3 cTransform::GetWorldLocation() const
{
	return glm::vec3(m_position);
}

glm::vec3 cTransform::GetEulerAngle() const
{
	glm::mat4 m = M();
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

const glm::mat4& cTransform::M() const
{
	glm::mat4 translationMatrix = GetTranslationMatrix();
	glm::mat4 rotationMatrix = GetRotationMatrix();
	glm::mat4 scalematrix = GetScaleMatrix();

	return rotationMatrix * scalematrix * translationMatrix;
}

const glm::mat4& cTransform::MInv() const
{
	return glm::inverse(M());
}

const glm::mat4 cTransform::TranspostInverse() const
{
	return glm::transpose(MInv());
}

const glm::mat4 cTransform::MirrorAccordingTo(const cTransform& t)
{
	glm::mat3 _mirror = glm::identity<glm::mat3>();
	_mirror[1][1] = -1;

	return M() * t.MInv()* glm::mat4(_mirror) * t.M();
}

bool cTransform::HasScale() const
{
#define NOT_ONE(x) ((x) < .999f || (x) > 1.001f)
	return (NOT_ONE(m_scale.x) || NOT_ONE(m_scale.y) || NOT_ONE(m_scale.z));
#undef NOT_ONE
}

void cTransform::Update()
{

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
