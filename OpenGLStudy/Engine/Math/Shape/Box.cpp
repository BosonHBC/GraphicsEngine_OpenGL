#include "Box.h"
#include "Sphere.h"

eCollisionType cBox::Intersect(const glm::vec3& i_point)
{
	if (i_point.x > b.x && i_point.y > b.y && i_point.z > b.z
		&& i_point.x < t.x && i_point.y < t.y && i_point.z < t.z) return ECT_Contain;
	else return ECT_NoIntersect;
}

eCollisionType cBox::Intersect(const cSphere* const i_sphere)
{
	double rr = i_sphere->r() * i_sphere->r(); // radius squared
	glm::vec3 sc = i_sphere->c(); // sphere center

	if (sc.x < b.x) rr -= pow((sc.x - b.x), 2);
	else if (sc.x > t.x) rr -= pow((sc.x - t.x),2);

	if (sc.y < b.y) rr -= pow((sc.y - b.y),2);
	else if (sc.y > t.y) rr -= pow((sc.y - t.y),2);

	if (sc.z < b.z) rr -= pow((sc.z -  b.z), 2);
	else if (sc.z > t.z) rr -= pow((sc.z - t.z), 2);

	if (rr > 0) return ECT_Overlap;
	else return ECT_NoIntersect;
}

cBox cBox::GetOctSubBox()
{
	glm::vec3 _center = Center();
	return cBox(_center + (b - _center) / 2.f, _center + (t - _center) / 2.f);
}

