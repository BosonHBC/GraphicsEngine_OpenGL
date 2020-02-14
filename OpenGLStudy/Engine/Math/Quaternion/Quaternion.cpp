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

cQuaternion cQuaternion::cQuaternion::Inverse() const
{
	return cQuaternion(w, -x, -y, -z);
}
