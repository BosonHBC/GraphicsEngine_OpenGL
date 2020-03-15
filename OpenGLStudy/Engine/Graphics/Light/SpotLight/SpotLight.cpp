#include "Graphics/Light/SpotLight/SpotLight.h"
#include <stdio.h>
#include "Graphics/Graphics.h"
#include "Graphics/UniformBuffer/UniformBufferFormats.h"
namespace Graphics {

	cSpotLight::cSpotLight(Color i_color, const glm::vec3& i_position, const glm::vec3& i_direction, GLfloat i_edge,  GLfloat i_radius):
		cPointLight(i_color, i_position, i_radius)
	{
		m_edge = glm::clamp(i_edge, 1.f, 150.f);
		m_procEdge = cosf(glm::radians(m_edge/2.f));
		glm::quat rot(glm::quatLookAt(i_direction, cTransform::WorldUp));
		m_transform.SetTransform(i_position, rot, glm::vec3(1, 1, 1));
	}

	void cSpotLight::Illuminate()
	{
		auto& gLighting = Graphics::GetGlobalLightingData();
		gLighting.spotLights[m_lightIndex].base.base.color = m_color;
		gLighting.spotLights[m_lightIndex].base.base.enableShadow = m_enableShadow;
		gLighting.spotLights[m_lightIndex].base.position = m_transform.Position();
		gLighting.spotLights[m_lightIndex].base.radius = m_range;
		gLighting.spotLights[m_lightIndex].direction = m_transform.Forward();
		gLighting.spotLights[m_lightIndex].edge = m_procEdge;
	}

	void cSpotLight::SetupLight(const GLuint& i_programID, GLuint i_lightIndex /*= 0*/)
	{
		cGenLight::SetupLight(i_programID, i_lightIndex);
		char _charBuffer[64] = { '\0' };

		snprintf(_charBuffer, sizeof(_charBuffer), "spotlightTransform[%d]", m_lightIndex);
		m_lightTransformID = glGetUniformLocation(i_programID, _charBuffer);

		snprintf(_charBuffer, sizeof(_charBuffer), "spotlightShadowMap[%d]", m_lightIndex);
		m_lightShadowMapID = glGetUniformLocation(i_programID, _charBuffer);
	}

	void cSpotLight::UseShadowMap(GLuint i_textureUnit)
	{
		glUniform1i(m_lightShadowMapID, i_textureUnit);
	}

	void cSpotLight::CreateShadowMap(GLuint i_width, GLuint i_height)
	{
		cGenLight::CreateShadowMap(i_width, i_height);
		float _aspect = static_cast<GLfloat>(i_width) / static_cast<GLfloat>(i_height);
		m_lightPrjectionMatrix = glm::perspective(glm::radians(m_edge), _aspect, 1.f, m_range *2.f);
	}

	glm::mat4 cSpotLight::CalculateLightTransform() const
	{
		glm::mat4 view = glm::lookAt(m_transform.Position(), m_transform.Position() + m_transform.Forward(), cTransform::WorldUp);
		return m_lightPrjectionMatrix * view;
	}


	void cSpotLight::SetLightUniformTransform()
	{
		glm::mat4 lightTransform = CalculateLightTransform();
		glUniformMatrix4fv(m_lightTransformID, 1, GL_FALSE, glm::value_ptr(lightTransform));
		assert(GL_NO_ERROR == glGetError());
	}

}
