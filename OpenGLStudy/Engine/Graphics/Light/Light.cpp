#include "Graphics/Light/Light.h"

namespace Graphics {

	cGenLight::cGenLight(): m_color(Color::White()), m_colorID(0)
	{
		m_transform = new cTransform();
	}

	cGenLight::cGenLight(Color i_color): m_color(i_color)
	{
		m_transform = new cTransform();
	}

	cGenLight::cGenLight(const cGenLight& i_other)
	{
		m_color = i_other.m_color;
		m_colorID = i_other.m_colorID;
		m_lightIndex = i_other.m_lightIndex;
		m_transform = new cTransform(*i_other.m_transform);
	}

	cGenLight& cGenLight::operator=(const cGenLight& i_other)
	{
		m_color = i_other.m_color;
		m_colorID = i_other.m_colorID;
		m_lightIndex = i_other.m_lightIndex;
		m_transform = new cTransform(*i_other.m_transform);
		return *this;
	}

	cGenLight::~cGenLight()
	{
		safe_delete(m_transform);
	}

	void cGenLight::SetupLight(const GLuint& i_programID, GLuint i_lightIndex)
	{
		m_lightIndex = (i_lightIndex < MAX_COUNT_PER_LIGHT)? i_lightIndex : MAX_COUNT_PER_LIGHT-1;
	}

}
