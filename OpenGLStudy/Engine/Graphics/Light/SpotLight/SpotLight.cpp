#include "Graphics/Light/SpotLight/SpotLight.h"
#include <stdio.h>

namespace Graphics {

	void cSpotLight::SetSpotLightInitialLocation(glm::vec3 i_pos, glm::vec3 i_dir)
	{
		cPointLight::SetLightInitialLocation(i_pos);
		m_dir = glm::normalize(i_dir);
	}

	void cSpotLight::Illuminate()
	{
		cPointLight::Illuminate();

		glUniform3f(m_directionID, m_dir.x, m_dir.y, m_dir.z);
		glUniform1f(m_edgeID, m_procEdge);
	}

	void cSpotLight::SetupLight(const GLuint& i_programID, GLuint i_lightIndex /*= 0*/)
	{
		cGenLight::SetupLight(i_programID, i_lightIndex);

		char _buff[100] = { '\0' };

		snprintf(_buff, sizeof(_buff), "spotLights[%d].base.base.color", m_lightIndex);
		m_colorID = glGetUniformLocation(i_programID, _buff);

		snprintf(_buff, sizeof(_buff), "spotLights[%d].base.position", m_lightIndex);
		m_positionID = glGetUniformLocation(i_programID, _buff);

		snprintf(_buff, sizeof(_buff), "spotLights[%d].base.constant", m_lightIndex);
		m_constID = glGetUniformLocation(i_programID, _buff);

		snprintf(_buff, sizeof(_buff), "spotLights[%d].base.linear", m_lightIndex);
		m_linearID = glGetUniformLocation(i_programID, _buff);

		snprintf(_buff, sizeof(_buff), "spotLights[%d].base.quadratic", m_lightIndex);
		m_quadraticID = glGetUniformLocation(i_programID, _buff);

		// Spot light stuffs

		snprintf(_buff, sizeof(_buff), "spotLights[%d].direction", m_lightIndex);
		m_directionID = glGetUniformLocation(i_programID, _buff);

		snprintf(_buff, sizeof(_buff), "spotLights[%d].edge", m_lightIndex);
		m_edgeID = glGetUniformLocation(i_programID, _buff);
	}

}
