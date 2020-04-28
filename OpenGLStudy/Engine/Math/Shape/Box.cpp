#include "Box.h"
#include "Sphere.h"

glm::vec3& cAABB::operator[](int i)
{
	assert(i >= 0 && i <= 1);
	if (i == 0) return this->b;
	else return this->t;
}

eCollisionType cAABB::Intersect(const glm::vec3& i_point) const
{
	if (i_point.x >= b.x && i_point.y >= b.y && i_point.z >= b.z
		&& i_point.x <= t.x && i_point.y <= t.y && i_point.z <= t.z) return ECT_Contain;
	else return ECT_NoIntersect;
}

eCollisionType cAABB::Intersect(const cSphere* const i_sphere) const
{
	double rr = i_sphere->r() * i_sphere->r(); // radius squared
	glm::vec3 sc = i_sphere->c(); // sphere center

	if (sc.x < b.x) rr -= pow((sc.x - b.x), 2);
	else if (sc.x > t.x) rr -= pow((sc.x - t.x), 2);

	if (sc.y < b.y) rr -= pow((sc.y - b.y), 2);
	else if (sc.y > t.y) rr -= pow((sc.y - t.y), 2);

	if (sc.z < b.z) rr -= pow((sc.z - b.z), 2);
	else if (sc.z > t.z) rr -= pow((sc.z - t.z), 2);

	if (rr > 0) return ECT_Overlap;
	else return ECT_NoIntersect;
}

float cAABB::NDF(const glm::vec3& i_point) const
{
	glm::vec3 _center = center();

	return 0;
}

glm::vec3 cAABB::corner(int i_corner)
{
	return glm::vec3(
		(*this)[(i_corner & 1)].x,
		(*this)[(i_corner & 2) ? 1 : 0].y,
		(*this)[(i_corner & 4) ? 1 : 0].z
	);
}

cAABB cAABB::getOctSubBox()
{
	glm::vec3 _center = center();
	return cAABB(_center + (b - _center) / 2.f, _center + (t - _center) / 2.f);
}

#include "stdio.h"
void cAABB::PrintCenterAndWidth() const
{
	glm::vec3 _center = center();
	printf("center: %d, %d, %d; width: %d.\n", (int)_center.x, (int)_center.y, (int)_center.z, (int)(t.x - b.x));
}

