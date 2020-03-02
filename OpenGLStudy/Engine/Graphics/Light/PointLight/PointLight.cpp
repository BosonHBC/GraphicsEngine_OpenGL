#include "PointLight.h"
#include <stdio.h>

#include "Graphics/Graphics.h"
#include "Graphics/UniformBuffer/UniformBufferFormats.h"
namespace Graphics {
	cPointLight::cPointLight(Color i_color, const glm::vec3 & i_position, const GLfloat i_range, GLfloat i_const, GLfloat i_linear, GLfloat i_quadratic):
		m_range(i_range), m_const(i_const), m_linear(i_linear), m_quadratic(i_quadratic),
		cGenLight(i_color)
	{
		m_transform->SetTransform(i_position, glm::quat(1, 0, 0, 0), glm::vec3(1, 1, 1));
	}
	void cPointLight::Illuminate()
	{
		auto& gLighting = Graphics::GetGlobalLightingData();
		gLighting.pointLights[m_lightIndex].base.color = m_color;
		gLighting.pointLights[m_lightIndex].base.enableShadow = m_enableShadow;
		gLighting.pointLights[m_lightIndex].position = m_transform->Position();
		gLighting.pointLights[m_lightIndex].quadratic = m_quadratic;
		gLighting.pointLights[m_lightIndex].linear = m_linear;
		gLighting.pointLights[m_lightIndex].constant = m_const;
	}

	void cPointLight::SetupLight(const GLuint& i_programID, GLuint i_lightIndex)
	{
		cGenLight::SetupLight(i_programID, i_lightIndex);

	}

}