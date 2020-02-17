#include "Graphics/Light/Light.h"
#include "Graphics/FrameBuffer/cFrameBuffer.h"

namespace Graphics {

	cGenLight::cGenLight(): m_color(Color::White()), m_colorID(0)
	{
		m_transform = new cTransform();
	}

	cGenLight::cGenLight(Color i_color): m_color(i_color)
	{
		m_transform = new cTransform();
	}



	void cGenLight::SetupLight(const GLuint& i_programID, GLuint i_lightIndex)
	{
		m_lightIndex = (i_lightIndex < MAX_COUNT_PER_LIGHT)? i_lightIndex : MAX_COUNT_PER_LIGHT-1;
	}

	void cGenLight::CleanUp()
	{
		safe_delete(m_transform);
		safe_delete(m_shadowMap);
	}

	void cGenLight::CreateShadowMap(GLuint i_width, GLuint i_height)
	{
		m_shadowMap = new cFrameBuffer();
		m_shadowMap->Initialize(i_width, i_height, ETextureType::ETT_FRAMEBUFFER_SHADOWMAP);
	}

	bool cGenLight::IsShadowMapValid() const
	{
		return m_shadowMap->IsValid();
	}

}
