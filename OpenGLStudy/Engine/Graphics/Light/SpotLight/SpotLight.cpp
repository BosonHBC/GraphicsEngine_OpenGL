#include "Graphics/Light/SpotLight/SpotLight.h"
#include <stdio.h>
#include "Graphics/Graphics.h"
#include "Graphics/UniformBuffer/UniformBufferFormats.h"
namespace Graphics {

	void cSpotLight::SetSpotLightInitialLocation(glm::vec3 i_pos, glm::vec3 i_dir)
	{
		cPointLight::SetLightInitialLocation(i_pos);
		m_dir = glm::normalize(i_dir);
	}

	void cSpotLight::Illuminate()
	{
		auto& gLighting = Graphics::GetGlobalLightingData();
		gLighting.spotLights[m_lightIndex].base.base.color = m_color;
		gLighting.spotLights[m_lightIndex].base.base.enableShadow = m_enableShadow;
		gLighting.spotLights[m_lightIndex].base.position = m_transform->GetWorldLocation();
		gLighting.spotLights[m_lightIndex].base.quadratic = m_quadratic;
		gLighting.spotLights[m_lightIndex].base.linear = m_linear;
		gLighting.spotLights[m_lightIndex].base.constant = m_const;
		gLighting.spotLights[m_lightIndex].direction = m_dir;
		gLighting.spotLights[m_lightIndex].edge = m_edge;
	}

	void cSpotLight::SetupLight(const GLuint& i_programID, GLuint i_lightIndex /*= 0*/)
	{
		cGenLight::SetupLight(i_programID, i_lightIndex);

	}

}
