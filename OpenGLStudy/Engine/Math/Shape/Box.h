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
	~cBox() {};

	eCollisionType Intersect(const glm::vec3& i_point) override;
	eCollisionType Intersect(const cSphere* const i_sphere);

	glm::vec3 b, t; // b and t stand for bottom corner and top corner

	glm::vec3 Center() const { return (b + t) / 2.f }
	cBox GetOctSubBox();

private:

};