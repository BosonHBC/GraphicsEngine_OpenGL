#include "Graphics/Light/SpotLight/SpotLight.h"
#include <stdio.h>
#include "Graphics/Graphics.h"
#include "Graphics/UniformBuffer/UniformBufferFormats.h"
namespace Graphics {

	cSpotLight::cSpotLight(Color i_color, const glm::vec3& i_position, const glm::vec3& i_direction, GLfloat i_edge, GLfloat i_range, GLfloat i_const, GLfloat i_linear, GLfloat i_quadratic):
		m_edge(i_edge),
		cPointLight(i_color, i_position, i_range, i_const, i_linear, i_quadratic)
	{
		m_procEdge = cosf(glm::radians(m_edge));
		glm::vec3 _right = glm::normalize(glm::cross(i_direction, cTransform::WorldUp));
		glm::vec3 _up = glm::normalize(glm::cross(_right, i_direction));
		glm::quat rot(glm::quatLookAt(i_direction, _up));
		m_transform->SetTransform(i_position, rot, glm::vec3(1, 1, 1));

	}

	void cSpotLight::Illuminate()
	{
		auto& gLighting = Graphics::GetGlobalLightingData();
		gLighting.spotLights[m_lightIndex].base.base.color = m_color;
		gLighting.spotLights[m_lightIndex].base.base.enableShadow = m_enableShadow;
		gLighting.spotLights[m_lightIndex].base.position = m_transform->Position();
		gLighting.spotLights[m_lightIndex].base.quadratic = m_quadratic;
		gLighting.spotLights[m_lightIndex].base.linear = m_linear;
		gLighting.spotLights[m_lightIndex].base.constant = m_const;
		gLighting.spotLights[m_lightIndex].direction = Transform()->Forward();
		gLighting.spotLights[m_lightIndex].edge = m_procEdge;
	}

	void cSpotLight::SetupLight(const GLuint& i_programID, GLuint i_lightIndex /*= 0*/)
	{
		cGenLight::SetupLight(i_programID, i_lightIndex);

		m_spotLightTransformID = glGetUniformLocation(i_programID, "spotlightTransform");
		m_spotLightShadowMapID = glGetUniformLocation(i_programID, "spotlightShadowMap");
	}

	void cSpotLight::CreateShadowMap(GLuint i_width, GLuint i_height)
	{
		cGenLight::CreateShadowMap(i_width, i_height);
		float _aspect = static_cast<GLfloat>(i_width) / static_cast<GLfloat>(i_height);
		m_lightPrjectionMatrix = glm::perspective(m_edge/2.f, _aspect, 0.1f, 1500.f);
	}

	glm::mat4 cSpotLight::CalculateLightTransform() const
	{
		return m_lightPrjectionMatrix * glm::lookAt(m_transform->Position(), m_transform->Position() + m_transform->Forward() , glm::vec3(0, 1, 0));
	}

	void cSpotLight::SetLightUniformTransform()
	{
		glUniformMatrix4fv(m_spotLightTransformID, 1, GL_FALSE, glm::value_ptr(CalculateLightTransform()));
	}

}
