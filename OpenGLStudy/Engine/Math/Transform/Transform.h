#pragma once
#include "GL/glew.h"
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"

class cTransform
{
public:
	/** Constructors&destructor and assignment operators*/
	cTransform(): m(glm::identity<glm::mat4>()), mInv(glm::identity<glm::mat4>()) {}
	cTransform(const glm::vec3& i_initialTranslation, const glm::quat& i_intialRotation, const glm::vec3& i_initialScale);
	cTransform(const cTransform& i_other) : m(i_other.m), mInv(i_other.mInv) {}
	cTransform(const glm::mat4& i_m) :m(i_m), mInv(glm::inverse(m)) {}
	cTransform(const glm::mat4& i_m, const glm::mat4& i_mInv) :m(i_m), mInv(i_mInv) {}
	cTransform& operator = (const cTransform& i_other);
	cTransform& operator = (const glm::mat4& i_m);
	~cTransform();


	/** static functions*/
	static glm::quat ToQuaternian(const double yaw, const double pitch, const double roll);
/*
	static cTransform Inverse(const cTransform& t) { return cTransform(t.mInv, t.m); }
	static cTransform Transpose(const cTransform& t) { return cTransform(transpose(t.m), transpose(t.mInv)); }
*/

	/** Usage function*/
	void Translate(const glm::vec3& i_location);
	// Local transformation
	void Rotate(const glm::vec3& i_axis, const float& i_angle);
	void Scale(const glm::vec3& i_scale);
	// Local transformation
	void gRotate(const glm::vec3& i_axis, const float& i_angle);
	void gScale(const glm::vec3& i_scale);
	
	/** Setters */
	void SetTransform(const glm::vec3& i_initialTranslation, const glm::quat& i_intialRotation, const glm::vec3& i_initialScale);
	void SetRotation(const glm::quat& i_rotation) { m_rotation = i_rotation; };
	/** Getters */
	glm::vec3 GetWorldLocation() const;
	glm::vec3 GetEulerAngle() const;
	glm::mat4 GetTranslationMatrix() const;
	glm::mat4 GetRotationMatrix() const;
	glm::mat4 GetScaleMatrix() const;
	const glm::vec3& Position() const { return m_position; }
	const glm::quat& Rotation() const { return m_rotation; }
	const glm::vec3& Scale() const { return m_scale; }
	glm::vec3 Forward() const;
	glm::vec3 Right() const;
	glm::vec3 Up() const;

	const glm::mat4& M() const { return m; }
	const glm::mat4& MInv() const { return mInv; }
	const glm::mat4 TranspostInverse() const { return transpose(mInv); }

	/** Helper functions*/
	bool HasScale() const;
	void Update();

#ifdef _DEBUG
	void PrintEulerAngle() const;
#endif // _DEBUG

public:
	static glm::vec3 WorldUp;
	static glm::vec3 WorldRight;
	static glm::vec3 WorldForward;

private:
	/** private data*/
	// for simplicity, m is the abbr of m_m, and so is mInv for m_mInv;
	glm::mat4 m, mInv;

	glm::vec3 m_position;
	glm::quat m_rotation;
	glm::vec3 m_scale;
};

