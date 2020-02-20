#include "PointLight.h"
#include <stdio.h>

#include "Graphics/Graphics.h"
#include "Graphics/UniformBuffer/UniformBufferFormats.h"
namespace Graphics {

	void cPointLight::Illuminate()
	{
		auto& gLighting = Graphics::GetGlobalLightingData();
		gLighting.pointLights[m_lightIndex].base.color = m_color;
		gLighting.pointLights[m_lightIndex].base.enableShadow = m_enableShadow;
		gLighting.pointLights[m_lightIndex].position = m_transform->GetWorldLocation();
		gLighting.pointLights[m_lightIndex].quadratic = m_quadratic;
		gLighting.pointLights[m_lightIndex].linear = m_linear;
		gLighting.pointLights[m_lightIndex].constant = m_const;
	}

	void cPointLight::SetupLight(const GLuint& i_programID, GLuint i_lightIndex)
	{
		cGenLight::SetupLight(i_programID, i_lightIndex);

	}

	void cPointLight::SetLightInitialLocation(glm::vec3 i_position)
	{
		m_transform->Translate(i_position);
	}

}