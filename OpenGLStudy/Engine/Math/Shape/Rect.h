#pragma once
#include "glm/glm.hpp"
struct sRect
{
	// bottom left
	glm::vec2 Min = glm::vec2(0.0f, 0.0f);
	// top right
	glm::vec2 Max = glm::vec2(0.0f, 0.0f);

	sRect() {}
	sRect(const sRect& i_other) : Min(i_other.Min), Max(i_other.Max) {}
	~sRect() {}
	sRect& operator = (const sRect& i_rhs) { Min = i_rhs.Min; Max = i_rhs.Max; return *this; }
	sRect(const glm::vec2& i_min, const glm::vec2& i_max) : Min(i_min), Max(i_max) 
	{
		assert(Max.x > Min.x);
		assert(Max.y > Min.y);
	}
	sRect(float minx, float miny, float maxx, float maxy) : Min(glm::vec2(minx, miny)), Max(glm::vec2(maxx, maxy)) {}
	sRect(const glm::vec2 i_min, float width, float height) : Min(i_min), Max(i_min + glm::vec2(width, height)) {}

	glm::vec2 center() const { return (Min + Max) / 2.f; }
	glm::vec2 br() const { return glm::vec2(Max.x, Min.y); }
	glm::vec2 tl() const { return glm::vec2(Min.x, Max.y); }
	float w() const { return Max.x - Min.x; }
	float h() const { return Max.y - Min.y; }
	float size() const { return glm::distance(Min, Max); }
	float area() const { return w() * h(); }
};