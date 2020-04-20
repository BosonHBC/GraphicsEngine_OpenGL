#pragma once
#include "Shape.h"

class cSphere : public cShape
{
public:
	cSphere() :m_radius(0), m_center(0) {};
	~cSphere() {};

	cSphere(const glm::vec3& i_center, float i_radius) : m_center(i_center), m_radius(i_radius) {}
	cSphere(const cSphere& i_other) : m_radius(i_other.m_radius), m_center(i_other.m_center) {}
	cSphere& operator = (const cSphere& i_other) { m_radius = i_other.m_radius; m_center = i_other.m_center; return *this; }
	
	eCollisionType Intersect(const glm::vec3& i_point) const override;
	eCollisionType Intersect(const cSphere& i_sphere) const ;
	eCollisionType IntersectRay(const glm::vec3& o, const glm::vec3& d, float& t1, float& t2) const;

	float NDF(const glm::vec3& i_point) const override;
	void SetRadius(float i_radius) { m_radius = i_radius; }
	void SetCenter(const glm::vec3& i_center) { m_center = i_center; }
	float r() const { return m_radius; }
	glm::vec3 c() const { return m_center; }
private:
	float m_radius;
	glm::vec3 m_center;
};
