#include "Quaternion.h"

cQuaternion::cQuaternion(const glm::vec3& v, float a)
{
	a = glm::radians(a);

	float aby2 = a / 2.f;

	w = cos(aby2);
	x = v.x* glm::sin(aby2);
	y = v.y * glm::sin(aby2);
	z = v.z * glm::sin(aby2);
}


glm::vec3 cQuaternion::operator*(const glm::vec3& i_rhs) const
{
	glm::vec3 vcV = glm::cross(glm::vec3(x, y, z), i_rhs);
	return i_rhs + vcV * (2.f * w) + 2.f * glm::cross(glm::vec3(x, y, z), vcV);
}

cQuaternion cQuaternion::operator*(const cQuaternion& i_rhs) const
{
	// q = qa * qb
	// q = [w * w' - v * v', w' * v + w * v' + v x v']
	glm::vec3 vl = glm::vec3(x, y, z);
	glm::vec3 vr = glm::vec3(i_rhs.x, i_rhs.y, i_rhs.z);

	return cQuaternion(w * i_rhs.w - glm::dot(vl, vr), i_rhs.w * vl + w * vr + glm::cross(vr, vl));
}

cQuaternion cQuaternion::cQuaternion::Inverse() const
{
	return cQuaternion(w, -x, -y, -z);
}
