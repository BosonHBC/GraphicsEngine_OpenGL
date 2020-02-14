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

	/** public functions*/
	cQuaternion Inverse() const;

public:
	/** public parameters*/
	float w, x, y, z;

private:
};