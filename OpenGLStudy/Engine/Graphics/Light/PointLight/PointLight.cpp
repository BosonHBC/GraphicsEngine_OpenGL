#include "PointLight.h"
#include <stdio.h>
namespace Graphics {
	

	void cPointLight::Illuminate()
	{
		glUniform3f(m_colorID, m_color.r, m_color.g, m_color.b);
		glUniform3f(m_positionID, m_position.x, m_position.y, m_position.z);
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

		snprintf(_buff, sizeof(_buff), "pointLights[%d].position", m_lightIndex);
		m_positionID = glGetUniformLocation(i_programID, _buff);

		snprintf(_buff, sizeof(_buff), "pointLights[%d].constant", m_lightIndex);
		m_constID = glGetUniformLocation(i_programID, _buff);

		snprintf(_buff, sizeof(_buff), "pointLights[%d].linear", m_lightIndex);
		m_linearID = glGetUniformLocation(i_programID, _buff);

		snprintf(_buff, sizeof(_buff), "pointLights[%d].quadratic", m_lightIndex);
		m_quadraticID = glGetUniformLocation(i_programID, _buff);

	}

	void cPointLight::SetupLight(glm::vec3 i_position)
	{
		m_position = i_position;
	}

}