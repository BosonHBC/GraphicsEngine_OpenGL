#pragma once
#include "glm/glm.hpp"
class cQuaternion
{
public:
	// identity quaternion
	cQuaternion() : w(1), x(0), y(0), z(0) {}
	cQuaternion(const cQuaternion& i_other)
		: w(i_other.w), x(i_other.x), y(i_other.y), z(i_other.z) {}
	cQuaternion& operator = (const cQuaternion& i_other)
	{
		w = i_other.w; x = i_other.x; y = i_other.y; z = i_other.z; return *this;
	}

	~cQuaternion() { w = x = y = z = 0; }

	// v: rotation angle, 
	// a: rotation angle in degree
	cQuaternion(const glm::vec3& v, float a);

	// generic constructor
	cQuaternion(float _w, float _x, float _y, float _z) : w(_w), x(_x), y(_y), z(_z) {}
	cQuaternion(float _w, const glm::vec3& i_v) : w(_w), x(i_v.x), y(i_v.y), z(i_v.z) {}

	/** public functions*/
	cQuaternion inverse() const;
	float length() const;
	cQuaternion normalized() const;

	/** Rotation matrix*/
	glm::mat4 ToRotationMatrix() const;


	glm::vec3 forward() const;
	glm::vec3 right() const;
	glm::vec3 up() const;

	cQuaternion operator * (const cQuaternion& i_rhs) const;
	cQuaternion& operator *= (const cQuaternion& i_rhs);
	glm::vec3 operator * (const glm::vec3& i_rhs) const;
	cQuaternion operator * (const float i_rhs) const;

public:
	/** public parameters*/
	float w, x, y, z;

private:
};