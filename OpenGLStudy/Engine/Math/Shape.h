#pragma once
#include "glm/glm.hpp"

enum eCollisionType : uint8_t
{
	ECT_NoIntersect = 0,
	ECT_Contain = 1,
	ECT_Overlap = 2,
	ECT_Tangency = 3,
};
#define TangencyPrecision 0.0001f

class cShape
{
public:
	cShape() {};
	~cShape() {};

	// Intersection between points
	virtual eCollisionType Intersect(const glm::vec3& i_point) = 0;

	static const char* CollisionEnumToString(const eCollisionType& i_collisionType);
private:

};