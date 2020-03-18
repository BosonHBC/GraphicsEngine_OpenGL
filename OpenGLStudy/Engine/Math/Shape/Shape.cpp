#include "Shape.h"
static const char * EnumStrings[] = { "ECT_NoIntersect", "ECT_Contain", "ECT_Overlap", "ECT_Tangency" };

const char* cShape::CollisionEnumToString(const eCollisionType& i_collisionType)
{
	return EnumStrings[i_collisionType];
}
