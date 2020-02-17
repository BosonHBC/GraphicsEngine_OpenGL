#include "PointLight.h"
#include <stdio.h>
namespace Graphics {

	void cPointLight::Illuminate()
	{
		glUniform3f(m_colorID, m_color.r, m_color.g, m_color.b);
		glUniform1i(m_enableShadowID, m_enableShadow);
		glm::vec3 worldLoc = m_transform->GetWorldLocation();
		glUniform3f(m_positionID, worldLoc.x, worldLoc.y, worldLoc.z);
		glUniform1f(m_constID, m_const);
		glUniform1f(m_linearID, m_linear);
		glUniform1f(m_quadraticID, m_quadratic);
	}

	void cPointLight::SetupLight(const GLuint& i_programID, GLuint i_lightIndex)
	{
		cGenLight::SetupLight(i_programID, i_lightIndex);

		char _buff[100] = { '\0' };

		snprintf(_buff, sizeof(_buff), "pointLights[%d].base.color", m_lightIndex);
		m_colorID = glGetUniformLocation(i_programID, _buff);

		snprintf(_buff, sizeof(_buff), "pointLights[%d].base.enableShadow", m_lightIndex);
		m_enableShadowID = glGetUniformLocation(i_programID, _buff);

		snprintf(_buff, sizeof(_buff), "pointLights[%d].position", m_lightIndex);
		m_positionID = glGetUniformLocation(i_programID, _buff);

		snprintf(_buff, sizeof(_buff), "pointLights[%d].constant", m_lightIndex);
		m_constID = glGetUniformLocation(i_programID, _buff);

		snprintf(_buff, sizeof(_buff), "pointLights[%d].linear", m_lightIndex);
		m_linearID = glGetUniformLocation(i_programID, _buff);

		snprintf(_buff, sizeof(_buff), "pointLights[%d].quadratic", m_lightIndex);
		m_quadraticID = glGetUniformLocation(i_programID, _buff);

	}

	void cPointLight::SetLightInitialLocation(glm::vec3 i_position)
	{
		m_transform->Translate(i_position);
	}

}