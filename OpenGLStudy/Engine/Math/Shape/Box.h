#pragma once
#include "Shape.h"

// forward declaration
class cSphere;

class cBox : public cShape
{
public:
	cBox() : b(-1e30), t(1e30) {};
	cBox(const glm::vec3& i_b, const glm::vec3& i_t) { b = i_b; t = i_t; }
	
	cBox(const cBox& i_ohter) : b(i_ohter.b), t(i_ohter.t) {}
	cBox& operator = (const cBox& i_ohter) { b = i_ohter.b; t = i_ohter.t; return *this; }

	cBox& operator += (const glm::vec3& i_rhs) { b += i_rhs; t += i_rhs; return *this; } // adding vector3 offset
	cBox operator + (const glm::vec3& i_rhs) { return cBox(b + i_rhs, t + i_rhs); } // adding vector3 offset
	glm::vec3& operator[] (int i);

	~cBox() {};

	eCollisionType Intersect(const glm::vec3& i_point) override;
	eCollisionType Intersect(const cSphere* const i_sphere);

	glm::vec3 b, t; // b and t stand for bottom corner and top corner

	glm::vec3 corner(int i_corner);
	glm::vec3 diagonal() const { return t - b; }
	glm::vec3 center() const { return (b + t) / 2.f; }
	cBox getOctSubBox();

	void PrintCenterAndWidth() const;

private:

};