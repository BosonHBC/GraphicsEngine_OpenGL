#pragma once
#include "GL/glew.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace Graphics {
	class cEffect;
}

class cTransform
{
public:
	/** Constructors&destructor and assignment operators*/
	cTransform(): m(glm::identity<glm::mat4>()), mInv(glm::identity<glm::mat4>()) {}
	cTransform(const cTransform& i_other) : m(i_other.m), mInv(i_other.mInv) {}
	cTransform(const glm::mat4& i_m) :m(i_m), mInv(glm::inverse(m)) {}
	cTransform(const glm::mat4& i_m, const glm::mat4& i_mInv) :m(i_m), mInv(i_mInv) {}
	cTransform& operator = (const cTransform& i_other);
	cTransform& operator = (const glm::mat4& i_m);
	~cTransform()
	{};

	/** static functions*/
	static cTransform Inverse(const cTransform& t) { return cTransform(t.mInv, t.m); }
	static cTransform Transpose(const cTransform& t) { return cTransform(transpose(t.m), transpose(t.mInv)); }

	/** Usage function*/
	void Translate(const glm::vec3& i_location);
	void Rotate(const glm::vec3& i_axis, const float& i_angle);
	void Scale(const glm::vec3& i_scale);

	/** Getters */
	glm::vec3 GetWorldLocation() const;
	const glm::mat4& M() const { return m; }
	const glm::mat4& MInv() const { return mInv; }

	/** Helper functions*/
	bool HasScale() const;
	void Update();
private:
	/** private data*/
	// for simplicity, m is the abbr of m_m, and so is mInv for m_mInv;
	glm::mat4 m, mInv;

};

