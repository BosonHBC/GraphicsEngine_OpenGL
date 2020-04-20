#include "Sphere.h"
#include "glm/gtx/norm.hpp"

eCollisionType cSphere::Intersect(const glm::vec3& i_point) const
{
	double rr = m_radius * m_radius;
	double dd = glm::distance2(m_center, i_point);

	double diff = dd - rr;
	if (diff > TangencyPrecision) return ECT_NoIntersect;
	else if (diff < -TangencyPrecision) return ECT_Contain;
	else return ECT_Tangency;
}

eCollisionType cSphere::Intersect(const cSphere& i_sphere) const
{
	double dd = glm::distance2(m_center, i_sphere.m_center);
	float r_sum = m_radius + i_sphere.m_radius;
	float r_sub = m_radius - i_sphere.m_radius;
	double r_sum_sqr = r_sum * r_sum;
	double r_sub_sqr = r_sub * r_sub;

	if (dd - r_sum_sqr > TangencyPrecision) return ECT_NoIntersect;
	else if (dd - r_sum_sqr < -TangencyPrecision && dd > r_sub_sqr) return ECT_Overlap;
	else if (dd <= r_sub_sqr) return ECT_Contain;
	else return ECT_Tangency;
}

eCollisionType cSphere::IntersectRay(const glm::vec3& o, const glm::vec3& d, float& t1, float& t2) const
{
	float A = glm::dot(d, d);
	float B = 2 * glm::dot(d, o - m_center);
	float C = glm::dot(o - m_center, o - m_center) - m_radius * m_radius;

	float DD = B * B - 4 * A*C;
	if (DD > 0) {
		t2 = (-B + sqrt(DD)) / (2 * A);
		t1 = (-B - sqrt(DD)) / (2 * A);
		return ECT_Overlap;
	}
	return ECT_NoIntersect;
}

float cSphere::NDF(const glm::vec3& i_point) const
{
	return glm::distance(m_center, i_point) / m_radius;
}

