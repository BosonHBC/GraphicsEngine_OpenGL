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


float cQuaternion::length() const
{
	return sqrt(x * x + y * y + z * z + w * w);
}

cQuaternion cQuaternion::normalized() const
{
	float _length = length();
	return cQuaternion(w / _length, x / _length, y / _length, z / _length);
}

glm::mat4 cQuaternion::ToRotationMatrix() const
{
	glm::vec3 f = forward();
	glm::vec3 u =up();
	glm::vec3 r = right();

	glm::mat4 m = glm::mat4(0);							
	m[0][0] = r.x;	m[1][0] = r.y;	m[2][0] = r.z;	m[3][0] = 0;
	m[0][1] = u.x;	m[1][1] = u.y;	m[2][1] = u.z;	m[3][1] = 0;
	m[0][2] = f.x;		m[1][2] = f.y;		m[2][2] = f.z;		m[3][2] = 0;
	// identity translation
	m[0][3] = 0;		m[1][3] = 0;		m[3][2] = 0;		m[3][3] = 1;

	return m;
}

glm::vec3 cQuaternion::forward() const
{
	return glm::vec3(2.0f * (x *  z - w * y), 2.0f * (y *  z + w * x), 1.0f - 2.0f * (x *  x + y * y));
}

glm::vec3 cQuaternion::right() const
{
	return glm::vec3(1.0f - 2.0f * (y *  y + z * z), 2.0f * (x *  y - w * z), 2.0f * (x *  z + w * y));
}

glm::vec3 cQuaternion::up() const
{
	return  glm::vec3(2.0f * (x *  y + w * z), 1.0f - 2.0f * (x *  x + z * z), 2.0f * (y *  z - w * x));
}



cQuaternion cQuaternion::operator*(const float i_rhs) const
{
	return cQuaternion(w* i_rhs, x*i_rhs, y*i_rhs, z*i_rhs);
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
cQuaternion& cQuaternion::operator*=(const cQuaternion& i_rhs)
{
	glm::vec3 vl = glm::vec3(x, y, z);
	glm::vec3 vr = glm::vec3(i_rhs.x, i_rhs.y, i_rhs.z);

	glm::vec3 _va = i_rhs.w * vl + w * vr + glm::cross(vr, vl);
	w = w * i_rhs.w - glm::dot(vl, vr);
	x = _va.x; y = _va.y;  z = _va.z;
	return *this;
}
cQuaternion cQuaternion::cQuaternion::inverse() const
{
	return cQuaternion(w, -x, -y, -z);
}
