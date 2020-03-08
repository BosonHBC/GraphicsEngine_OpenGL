#include "Graphics/Light/Light.h"
#include "Graphics/FrameBuffer/cFrameBuffer.h"

namespace Graphics {

	cGenLight::cGenLight(): m_color(Color::Black()),
		m_lightIndex(0), m_enableShadow(false),
		m_transform(cTransform()), m_lightPrjectionMatrix(glm::mat4(1)), m_shadowMap(nullptr)
	{
	}

	cGenLight::cGenLight(Color i_color): m_color(i_color)
	{
	}


	cGenLight::cGenLight(const cGenLight& i_other) :
		m_color(i_other.m_color), m_lightIndex(i_other.m_lightIndex), m_enableShadow(i_other.m_enableShadow),
		m_transform(i_other.m_transform), m_lightPrjectionMatrix(i_other.m_lightPrjectionMatrix)
	{
		// Let the light point to the same shadow map
		m_shadowMap = i_other.m_shadowMap;
	}

	cGenLight& cGenLight::operator=(const cGenLight& i_other)
	{
		if (&i_other != nullptr) {
			m_color = i_other.m_color;
			m_lightIndex = i_other.m_lightIndex;
			m_enableShadow = i_other.m_enableShadow;
			m_transform = i_other.m_transform;
			m_lightPrjectionMatrix = i_other.m_lightPrjectionMatrix;
			m_shadowMap = i_other.m_shadowMap;
			return *this;
		}
		else
		{
			*this = cGenLight();
			return *this;
		}
		
	}

	void cGenLight::SetupLight(const GLuint& i_programID, GLuint i_lightIndex)
	{
		m_lightIndex = (i_lightIndex < MAX_COUNT_PER_LIGHT)? i_lightIndex : MAX_COUNT_PER_LIGHT-1;
	}

	void cGenLight::UpdateLightIndex(GLuint i_lightIndex)
	{
		m_lightIndex = (i_lightIndex < MAX_COUNT_PER_LIGHT) ? i_lightIndex : MAX_COUNT_PER_LIGHT - 1;
	}

	void cGenLight::CleanUp()
	{
		m_transform.~cTransform();
	}

	void cGenLight::CreateShadowMap(GLuint i_width, GLuint i_height)
	{
		m_shadowMap = new cFrameBuffer();
		m_shadowMap->Initialize(i_width, i_height, ETextureType::ETT_FRAMEBUFFER_SHADOWMAP);
	}

	bool cGenLight::IsShadowMapValid() const
	{
		return (m_shadowMap != nullptr && m_shadowMap->IsValid());
	}

	void cGenLight::CleanUpShadowMap()
	{
		// Shadow map should be cleanup manually
		safe_delete(m_shadowMap);
	}

}
