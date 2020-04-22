#pragma once
#include "Shape.h"

// forward declaration
class cSphere;

class cAABB : public cShape
{
public:
	cAABB() : b(glm::vec3(-4096)), t(glm::vec3(4096)) {};
	cAABB(const glm::vec3& i_b, const glm::vec3& i_t) { b = i_b; t = i_t; }
	
	cAABB(const cAABB& i_ohter) : b(i_ohter.b), t(i_ohter.t) {}
	cAABB& operator = (const cAABB& i_ohter) { b = i_ohter.b; t = i_ohter.t; return *this; }

	cAABB& operator += (const glm::vec3& i_rhs) { b += i_rhs; t += i_rhs; return *this; } // adding vector3 offset
	cAABB operator + (const glm::vec3& i_rhs) { return cAABB(b + i_rhs, t + i_rhs); } // adding vector3 offset
	glm::vec3& operator[] (int i);

	~cAABB() {};

	eCollisionType Intersect(const glm::vec3& i_point)const override;
	float NDF(const glm::vec3& i_point) const override;
	eCollisionType Intersect(const cSphere* const i_sphere) const;

	glm::vec3 b, t; // b and t stand for bottom corner and top corner

	glm::vec3 corner(int i_corner);
	glm::vec3 diagonal() const { return t - b; }
	glm::vec3 center() const { return (b + t) / 2.f; }
	cAABB getOctSubBox();

	void PrintCenterAndWidth() const;

private:

};